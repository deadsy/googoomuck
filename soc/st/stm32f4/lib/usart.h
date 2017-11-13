//-----------------------------------------------------------------------------
/*

USART Driver

*/
//-----------------------------------------------------------------------------

#ifndef USART_H
#define USART_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

#define TXBUF_SIZE 128		// must be a power of 2
#define RXBUF_SIZE 128		// must be a power of 2

struct usart_cfg {
	uint32_t base;
	int baud;
};

struct usart_drv {
	USART_TypeDef *regs;
	int irq;
	uint8_t txbuf[TXBUF_SIZE];
	uint8_t rxbuf[RXBUF_SIZE];
	volatile int rx_wr, rx_rd;
	volatile int tx_wr, tx_rd;
	int rx_errors;
};

//-----------------------------------------------------------------------------

int usart_init(struct usart_drv *usart, struct usart_cfg *cfg);
void usart_putc(struct usart_drv *usart, char c);
void usart_flush(struct usart_drv *usart);
int usart_tstc(struct usart_drv *usart);
char usart_getc(struct usart_drv *usart);

//-----------------------------------------------------------------------------

#endif				// USART_H

//-----------------------------------------------------------------------------
