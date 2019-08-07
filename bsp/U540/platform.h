/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#ifndef HEXFIVE_PLATFORM_H
#define HEXFIVE_PLATFORM_H

#define CPU_FREQ	1000000000 //33300000
#define RTC_FREQ	  1000000
#define AXI_FREQ	 75000000 // ChipLink

// -----------------------------------------------------------------------------
// RTC (MTIME)
// -----------------------------------------------------------------------------
#define RTC_BASE	0x02000000
#define RTC_MTIME	    0xBFF8

// -----------------------------------------------------------------------------
// UART0
// -----------------------------------------------------------------------------

/*
#define UART0_BASE 	0x10010000

#define UART_TXFIFO 0x00
#define UART_RXFIFO 0x04
#define UART_TXCTRL 0x08
#define UART_RXCTRL 0x0c
#define UART_IE 	0x10
#define UART_IP 	0x14
#define UART_DIV 	0x18
*/

#define UART0_BASE 	0x2000104000 // ChipLink

#define UART_RBR 	0x00	// Receiver buffer register
#define UART_THR 	0x00	// Transmit holding register
#define UART_DLR	0x00	// Divisor latch (LSB)
#define UART_DRM	0x04	// Divisor latch (MSB)
#define UART_LCR 	0x0C	// Line control register
#define UART_FCR 	0x08	// FIFO control register
#define UART_LSR 	0x14	// Line status register

// -----------------------------------------------------------------------------
// PWM
// -----------------------------------------------------------------------------
#define PWM0_BASE	0x10020000

#define PWM_CFG		0x00	// PWM configuration register
#define PWM_COUNT	0x08	// PWM count register
#define PWM_SCOUNT	0x10	// PWM scaled count register
#define PWM_CMP0 	0x20	// D1 Green
#define PWM_CMP1 	0x24	// D2 Green
#define PWM_CMP2 	0x28	// D3 Green
#define PWM_CMP3 	0x2C	// D4 Green

// -----------------------------------------------------------------------------
// GPIO
// -----------------------------------------------------------------------------
#define GPIO_BASE 		0x10060000

#define GPIO_INPUT_VAL  0x00
#define GPIO_INPUT_EN   0x04
#define GPIO_OUTPUT_EN  0x08
#define GPIO_OUTPUT_VAL 0x0C
#define GPIO_PULLUP_EN  0x10
#define GPIO_DRIVE      0x14
#define GPIO_RISE_IE    0x18
#define GPIO_RISE_IP    0x1C
#define GPIO_FALL_IE    0x20
#define GPIO_FALL_IP    0x24
#define GPIO_HIGH_IE    0x28
#define GPIO_HIGH_IP    0x2C
#define GPIO_LOW_IE     0x30
#define GPIO_LOW_IP     0x34
#define GPIO_OUTPUT_XOR 0x40

// -----------------------------------------------------------------------------
// C Helper functions
// -----------------------------------------------------------------------------

#define _REG64(base, offset) (*(volatile uint64_t *)((base) + (offset)))
#define _REG32(base, offset) (*(volatile uint32_t *)((base) + (offset)))
#define _REG16(base, offset) (*(volatile uint16_t *)((base) + (offset)))

#define RTC_REG(offset) _REG64(RTC_BASE, offset)
#define UART0_REG(offset) _REG32(UART0_BASE, offset)
#define PWM0_REG(offset) _REG32(PWM0_BASE, offset)
#define GPIO_REG(offset) _REG32(GPIO_BASE, offset)

#define UART2_REG(offset) _REG32(UART2_BASE, offset) // ChipLink


#endif /* HEXFIVE_PLATFORM_H */

