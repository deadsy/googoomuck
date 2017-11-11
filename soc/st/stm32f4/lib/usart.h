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

struct usart_drv {
	USART_TypeDef *regs;
	void *priv;		// users private data
	void (*err_callback) (struct usart_drv * usart, uint32_t errors);	// errors callback
	void (*rx_callback) (struct usart_drv * usart, uint8_t c);	// rx callback
	int (*tx_callback) (struct usart_drv * usart, uint8_t * c);	// tx callback
};

struct usart_cfg {
	uint32_t base;
	int baud;
	void *priv;
	void (*err_callback) (struct usart_drv * usart, uint32_t errors);	// errors callback
	void (*rx_callback) (struct usart_drv * usart, uint8_t c);	// rx callback
	int (*tx_callback) (struct usart_drv * usart, uint8_t * c);	// tx callback
};

//-----------------------------------------------------------------------------

int usart_init(struct usart_drv *usart, struct usart_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// USART_H

//-----------------------------------------------------------------------------
