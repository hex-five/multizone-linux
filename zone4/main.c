/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#include <string.h> // strcmp() strlen()
#include <libhexfive.h>
#include <platform.h>
#include "owi_task.h"

#define SPI_TDI 3 	// in  - pin 14
#define SPI_TCK 2	// out - pin 13
#define SPI_TDO 6	// out - pin 9
#define LED4 0 		// green

uint8_t CRC8(uint8_t bytes[]){

    const uint8_t generator = 0x1D;
    uint8_t crc = 0;

    for(int b=0; b<3; b++) {

        crc ^= bytes[b]; /* XOR-in the next input byte */

        for (int i = 0; i < 8; i++)
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ generator);
            else
                crc <<= 1;
    }

    return crc;
}

uint32_t spi_rw(uint8_t cmd[]){

	uint32_t rx_data = 0;

	const uint32_t tx_data = ((uint8_t)cmd[0] << 24) |  ((uint8_t)cmd[1] << 16) | ((uint8_t)cmd[2] << 8) | CRC8(cmd);

	for (int i=32-1, bit; i>=0; i--){

		bit = (tx_data >> i) & 1U;
		GPIO_REG(GPIO_OUT) = (bit==1 ? GPIO_REG(GPIO_OUT) | (0x1 << SPI_TDO) :
											  GPIO_REG(GPIO_OUT) & ~(0x1 << SPI_TDO)  );

		GPIO_REG(GPIO_OUT) |= (0x1 << SPI_TCK); volatile int w1=0; while(w1<5) w1++;

		GPIO_REG(GPIO_OUT) ^= (0x1 << SPI_TCK); volatile int w2=0; while(w2<1) w2++;

		bit = ( GPIO_REG(GPIO_IN) >> SPI_TDI) & 1U;
		rx_data = ( bit==1 ? rx_data |  (0x1 << i) : rx_data & ~(0x1 << i) );

	}

	return rx_data;
}

int main (void){

	//volatile int w=0; while(1){w++;}
	//while(1) {ECALL_YIELD();}

	GPIO_REG(GPIO_3_CFG) = 0b010; // in
	GPIO_REG(GPIO_2_CFG) = 0b101; // out buffer
	GPIO_REG(GPIO_6_CFG) = 0b101; // out buffer
	GPIO_REG(GPIO_0_CFG) = 0b101; // out buffer

	PWM0_REG(PWM_CFG)    = 0x0;
	PWM0_REG(PWM_COUNT)  = 0x0;
	PWM0_REG(PWM_SCOUNT) = 0x0;

	PWM0_REG(PWM_CMP0) = 0x1; // LED1 OFF
	PWM0_REG(PWM_CMP1) = 0x1; // LED2 OFF
	PWM0_REG(PWM_CMP2) = 0x1; // LED3 OFF
	PWM0_REG(PWM_CMP3) = 0x1; // LED4 OFF

	#define CMD_STOP  ((uint8_t[]){0x00, 0x00, 0x00})
	#define CMD_DUMMY ((uint8_t[]){0xFF, 0xFF, 0xFF})
	#define CMD_TIME   250*RTC_FREQ/1000 // 250ms
	#define PING_TIME 1000*RTC_FREQ/1000 // 1000ms
	#define SYS_TIME  RTC_REG(RTC_MTIME)

	uint64_t ping_timer=0, cmd_timer=0;
	uint32_t rx_data = 0, usb_state = 0;

	while(1){

		// Message listener
		char msg[16]=""; int zone=-1;
		for (zone=1; zone<4; zone++)
			if (ECALL_RECV(zone, msg)) break;

		if (strlen(msg)){

			// Ping
			if (strcmp("ping", msg) == 0) ECALL_SEND(zone, "pong");

			// Manual cmd
			else if (usb_state==0x12670000 && cmd_timer==0){

				uint8_t cmd[3] = {0x00, 0x00, 0x00};

				switch (msg[0]){
					case 'q' : cmd[0] = 0x01; break; // grip close
					case 'a' : cmd[0] = 0x02; break; // grip open
					case 'w' : cmd[0] = 0x04; break; // wrist up
					case 's' : cmd[0] = 0x08; break; // wrist down
					case 'e' : cmd[0] = 0x10; break; // elbow up
					case 'd' : cmd[0] = 0x20; break; // elbow down
					case 'r' : cmd[0] = 0x40; break; // shoulder up
					case 'f' : cmd[0] = 0x80; break; // shoulder down
					case 't' : cmd[1] = 0x01; break; // base clockwise
					case 'g' : cmd[1] = 0x02; break; // base counterclockwise
					case 'y' : cmd[2] = 0x01; break; // light on
					default  : break;
				}

				if ( cmd[0] + cmd[1] + cmd[2] != 0 ){
					rx_data = spi_rw(cmd);
					cmd_timer = SYS_TIME + CMD_TIME;
					ping_timer = SYS_TIME + PING_TIME;
				}

			}

		}

		// Auto stop manual commands after CMD_TIME
	    if (cmd_timer >0 && SYS_TIME > cmd_timer){
	    	rx_data = spi_rw(CMD_STOP);
	    	cmd_timer=0;
	    	ping_timer = SYS_TIME + PING_TIME;
	    }

	    // Detect USB state every 1s & blink pwm led (Unleashed)
	    if (SYS_TIME > ping_timer){
	    	rx_data = spi_rw(CMD_DUMMY);
	    	PWM0_REG(PWM_CMP3) ^= 0x1; // CMP3 = LED4;
	    	ping_timer = SYS_TIME + PING_TIME;
	    }

	    // Update USB state & led on EXP board and broadcast usb messages
	    if (rx_data != usb_state){
	    	if (rx_data==0x12670000){
	    		for (int z=1; z<4; z++) ECALL_SEND(z, "USB DEV ATTACH");
	    		GPIO_REG(GPIO_OUT) |= 1<<LED4;
	    	} else if (rx_data==0x00000000){
	    		for (int z=1; z<4; z++)	ECALL_SEND(z, "USB DEV DETACH");
	    		GPIO_REG(GPIO_OUT) &=  ~(1<<LED4);
	    		owi_task_stop_request();
	    	}
	    	usb_state=rx_data;
	    }

		// OWI sequence
	    if (msg[0]=='<' && usb_state==0x12670000) owi_task_fold();
	    if (msg[0]=='>' && usb_state==0x12670000) owi_task_unfold();
		if (msg[0]=='1' && usb_state==0x12670000) owi_task_start_request();
		if (msg[0]=='0' || usb_state!=0x12670000) owi_task_stop_request();
		int32_t cmd = -1;
		if ( usb_state==0x12670000 && (cmd = owi_task_run(SYS_TIME)) != -1){
			rx_data = spi_rw((uint8_t[]){(uint8_t)cmd, (uint8_t)(cmd>>8), (uint8_t)(cmd>>16)});
			ping_timer = SYS_TIME + PING_TIME;
		}

		// Yield CPU to other zones
		ECALL_YIELD();

	} // while(1)

} // main()




