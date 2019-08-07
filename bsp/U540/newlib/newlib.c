/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include <platform.h>

#include <libhexfive.h>

//File descriptor
enum FD {
	UART_FD = 0,
	ZONE0_FD = 3,
	ZONE1_FD,
	ZONE2_FD,
	ZONE3_FD,
	ZONE4_FD,
	MAX_FD,
};

// ----------------------------------------------------------------------------
int _close(int file) {
// ----------------------------------------------------------------------------

	return -1;
}

// ----------------------------------------------------------------------------
int _fstat(int file, struct stat *st) {
// ----------------------------------------------------------------------------

	st->st_mode = S_IFCHR;
	return 0;
}

// ----------------------------------------------------------------------------
void * _sbrk(int incr) {
// ----------------------------------------------------------------------------

	extern char _end[];
	extern char _heap_end[];
	static char *_heap_ptr = _end;

	if ((_heap_ptr + incr < _end) || (_heap_ptr + incr > _heap_end))
		return  (void *) -1;

	_heap_ptr += incr;
	return _heap_ptr - incr;
}

// ----------------------------------------------------------------------------
int _isatty(int file) {
// ----------------------------------------------------------------------------

	return (file == STDIN_FILENO || file == STDOUT_FILENO || file == STDERR_FILENO
				|| file == ZONE0_FD || file == ZONE1_FD || file == ZONE2_FD || file == ZONE3_FD  
				|| file == ZONE4_FD ) ? 1 : 0;

}

struct _file {
    const char *name;
    uint32_t rp;
};

static struct _file files[MAX_FD] = {
	{
		.name = "UART",
	},
	{
		.name = "UART",
	},
	{
		.name = "UART",
	},
	{
		.name = "ZONE0",
	},
	{
		.name = "ZONE1",
	},
	{
		.name = "ZONE2",
	},
	{
		.name = "ZONE3",
	},
	{
		.name = "ZONE4",
	},
};

// ----------------------------------------------------------------------------
const char * _ttyname(int file) {
// ----------------------------------------------------------------------------

	if (file >= MAX_FD) {
		return NULL;
	} else {
		return files[file].name;
	}

}

// ----------------------------------------------------------------------------
int _lseek(int file, off_t ptr, int dir) {
// ----------------------------------------------------------------------------

	return 0;
}

// ----------------------------------------------------------------------------
int _open(const char* name, int flags, int mode) {
// ----------------------------------------------------------------------------

	if ( strcmp(name, files[UART_FD].name) == 0 ) {

		UART0_REG(UART_LCR) = 0b10000000; // Enable Divisor latch access
		UART0_REG(UART_DLR) = AXI_FREQ/16/115200;
		UART0_REG(UART_LCR) = 0b00000011; // Disable Divisor latch access & set Word length=8bit

		UART0_REG(UART_FCR) = 0b00000111; // Clear TX/RX FIFO

		return (enum FD) UART_FD;

	} else {

		for (int i = ZONE0_FD; i <= ZONE4_FD; i++) {
			if ( strcmp(name, files[i].name) == 0 ) {
				files[i].rp = 0;
				return i;
			}
		}

	}

	return -1;
}

// ----------------------------------------------------------------------------
int _read(int file, char *ptr, size_t len) {
// ----------------------------------------------------------------------------

	ssize_t count = 0;
	static uint32_t msg[4] = {0};
	int i = 0;

	if( _isatty(file) ) {

		switch (file){

		case UART_FD:
			
			while( count<len && ( UART0_REG(UART_LSR) & 1<<0 ) ){ // DR Data Ready
				*ptr++ = (char)UART0_REG(UART_RBR);
				count++;
			}

			return (count) ? count : -EAGAIN; 
			break;

		case ZONE0_FD:
		case ZONE1_FD:
		case ZONE2_FD:
		case ZONE3_FD:
		case ZONE4_FD:

			if (msg[0] == 0) {
				if (!ECALL_RECV((file-ZONE0_FD), msg))
					return -EAGAIN;
			}

			memcpy(ptr, &msg[0], 1);
			i = 1;
			msg[0] >>= 8;

			return i;
			break;
		
		default:

			/* Do nothing */
			break;
		}

	}	

	return -1;
}

// ----------------------------------------------------------------------------
size_t _write(int file, const void *ptr, size_t len) {
// ----------------------------------------------------------------------------

	uint8_t * buff = (uint8_t *)ptr;
	int i = 0;
	uint32_t msg[4];
 
	if( _isatty(file) ) {

		switch (file){

		case UART_FD:

			for (i = 0; i < len; i++) {

				while ( (UART0_REG(UART_LSR) & 1<<5) == 0){ECALL_YIELD();} // THRE Transmitter register empty

				UART0_REG(UART_THR) = buff[i];

				if (buff[i] == '\n') {
					while ( (UART0_REG(UART_LSR) & 1<<5) == 0){;} // THRE
					UART0_REG(UART_THR) = '\r';
				}

			}

			return len;
			break;

		case ZONE0_FD:
		case ZONE1_FD:
		case ZONE2_FD:
		case ZONE3_FD:
		case ZONE4_FD:

			/* Pack Buff into MZ messages */
			while (i < len) {
				int transfer = len - i;
				if (transfer > sizeof(msg))
					transfer = sizeof(msg);

				memset(msg, 0, sizeof(msg));
				/* Copy buff to msg (8-bit operations) */
				int real_transfer = 0;
				for(int j=0; j<transfer; j++, real_transfer++){
					*((int8_t *)(&msg[0])+j) = buff[real_transfer];
					/* In case of '\n' place also '\r' */
					if(buff[real_transfer] == '\n'){
						if( (j<transfer-1) || (transfer < sizeof(msg)) ){ // In the middle
							*((int8_t *)(&msg[0])+(j+1)) = '\r';
							j++;
						}else{ // In the last, need to pack on the next one
							real_transfer--;
						}
						
					}
				}

				if (ECALL_SEND((file-ZONE0_FD), msg)) {
					/* In the last, need to pack on the next but update buff */
					if(real_transfer<transfer){ 
						buff[real_transfer] = '\r'; 
					}
					i += real_transfer;
					buff += real_transfer;
				}
				ECALL_YIELD();

			}

			return i;
			break;
		
		default:

			/* Do nothing */
			break;
		}

	}

	return -1;
}

