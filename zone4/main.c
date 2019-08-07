/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#include <platform.h>
#include <libhexfive.h>

void trap_0x3_handler(void)__attribute__((interrupt("user")));
void trap_0x3_handler(void){

	PWM0_REG(PWM_CMP3) ^= 0x1; // CMP3 = LED4;

	ECALL_CSRW_MTIMECMP(ECALL_CSRR_MTIME() + 500*RTC_FREQ/1000);

}

int main (void){

	//volatile int w=0; while(1){w++;}
	//while(1) ECALL_YIELD();

	PWM0_REG(PWM_CFG)    = 0x0;
	PWM0_REG(PWM_COUNT)  = 0x0;
	PWM0_REG(PWM_SCOUNT) = 0x0;

	PWM0_REG(PWM_CMP0) = 0x1; // LED1 OFF
	PWM0_REG(PWM_CMP1) = 0x1; // LED2 OFF
	PWM0_REG(PWM_CMP2) = 0x1; // LED3 OFF
	PWM0_REG(PWM_CMP3) = 0x1; // LED4 OFF

	ECALL_TRP_VECT(0x3, trap_0x3_handler); // 0x3 Soft timer

	ECALL_CSRW_MTIMECMP(ECALL_CSRR_MTIME() + 500*RTC_FREQ/1000);

	int msg[4]={0,0,0,0}; int zone=1;

	while(1){

		if (ECALL_RECV(zone, msg)) ECALL_SEND(zone, msg);

		zone = 1 + zone%4;

		ECALL_YIELD();
	}

}
