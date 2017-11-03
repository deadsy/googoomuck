//-----------------------------------------------------------------------------
/*

USART Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// enable the clock to the usart module
static void usart_module_enable(uint32_t base) {
	if (base == USART1_BASE) {
		RCC->APB2ENR |= (1 << 4 /*USART1EN */ );
	} else if (base == USART2_BASE) {
		RCC->APB1ENR |= (1 << 17 /*USART2EN */ );
	} else if (base == USART3_BASE) {
		RCC->APB1ENR |= (1 << 18 /*USART3EN */ );
	} else if (base == UART4_BASE) {
		RCC->APB1ENR |= (1 << 19 /*USART4EN */ );
	} else if (base == UART5_BASE) {
		RCC->APB1ENR |= (1 << 20 /*USART5EN */ );
	} else if (base == USART6_BASE) {
		RCC->APB2ENR |= (1 << 5 /*USART6EN */ );
	}
}

//-----------------------------------------------------------------------------
// baud rate setting

static uint32_t usart_get_clk(struct usart_drv *usart) {
	uint32_t base = (uint32_t) usart->regs;
	uint32_t clk;
	if (base == USART1_BASE || base == USART6_BASE) {
		clk = HAL_RCC_GetPCLK2Freq();
	} else {
		clk = HAL_RCC_GetPCLK1Freq();
	}
	return clk;
}

static uint32_t usart_get_brr(uint32_t clk, int baud, int over) {
	uint32_t div = (16 * clk) / (over * baud);
	uint32_t frac = div & 15;
	if (over == 8) {
		frac >>= 1;	// div[3] == 0
	}
	return (div & ~15) | frac;
}

// set the baud rate
static void usart_set_baud(struct usart_drv *usart, int baud) {
	uint32_t clk = usart_get_clk(usart);
	uint32_t over = (usart->regs->CR1 & USART_CR1_OVER8) ? 8 : 16;
	reg_rmw(&usart->regs->BRR, 0xffff, usart_get_brr(clk, baud, over));
}

//-----------------------------------------------------------------------------

#define USART_SR_MASK (0x3ffU)
#define USART_CR1_MASK (0xbfffU)
#define USART_CR2_MASK (0x7f6fU)
#define USART_CR3_MASK (0xfffU)
#define USART_GTPR_MASK (0xffffU)

int usart_init(struct usart_drv *usart, struct usart_cfg *cfg) {
	uint32_t val;

	memset(usart, 0, sizeof(struct usart_drv));
	usart->regs = (USART_TypeDef *) cfg->base;

	// enable the usart module
	usart_module_enable(cfg->base);

	// Control register 1
	val = 0;
	val |= (0 << 15 /*OVER8 */ );	// Oversampling mode
	val |= (0 << 13 /*UE*/);	// USART enable
	val |= (0 << 12 /*M*/);	// Word length
	val |= (0 << 11 /*WAKE*/);	// Wakeup method
	val |= (0 << 10 /*PCE*/);	// Parity control enable
	val |= (0 << 9 /*PS*/);	// Parity selection
	val |= (0 << 8 /*PEIE*/);	// PE interrupt enable
	val |= (0 << 7 /*TXEIE*/);	// TXE interrupt enable
	val |= (0 << 6 /*TCIE*/);	// Transmission complete interrupt enable
	val |= (0 << 5 /*RXNEIE*/);	// RXNE interrupt enable
	val |= (0 << 4 /*IDLEIE*/);	// IDLE interrupt enable
	val |= (0 << 3 /*TE*/);	// Transmitter enable
	val |= (0 << 2 /*RE*/);	// Receiver enable
	val |= (0 << 1 /*RWU*/);	// Receiver wakeup
	val |= (0 << 0 /*SBK*/);	// Send break
	reg_rmw(&usart->regs->CR1, USART_CR1_MASK, val);

	// Control register 2
	val = 0;
	val |= (0 << 14 /*LINEN*/);	// LIN mode enable
	val |= (0 << 12 /*STOP*/);	// STOP bits
	val |= (0 << 11 /*CLKEN*/);	// Clock enable
	val |= (0 << 10 /*CPOL*/);	// Clock polarity
	val |= (0 << 9 /*CPHA*/);	// Clock phase
	val |= (0 << 8 /*LBCL*/);	// Last bit clock pulse
	val |= (0 << 6 /*LBDIE*/);	// LIN break detection interrupt enable
	val |= (0 << 5 /*LBDL*/);	// lin break detection length
	val |= (0 << 0 /*ADD*/);	// Address of the USART node
	reg_rmw(&usart->regs->CR2, USART_CR2_MASK, val);

	// Control register 3
	val = 0;
	val |= (0 << 11 /*ONEBIT*/);	// One sample bit method enable
	val |= (0 << 10 /*CTSIE*/);	// CTS interrupt enable
	val |= (0 << 9 /*CTSE*/);	// CTS enable
	val |= (0 << 8 /*RTSE*/);	// RTS enable
	val |= (0 << 7 /*DMAT*/);	// DMA enable transmitter
	val |= (0 << 6 /*DMAR*/);	// DMA enable receiver
	val |= (0 << 5 /*SCEN*/);	// Smartcard mode enable
	val |= (0 << 4 /*NACK*/);	// Smartcard NACK enable
	val |= (0 << 3 /*HDSEL*/);	// Half-duplex selection
	val |= (0 << 2 /*IRLP*/);	// IrDA low-power
	val |= (0 << 1 /*IREN*/);	// IrDA mode enable
	val |= (0 << 0 /*EIE*/);	// Error interrupt enable
	reg_rmw(&usart->regs->CR3, USART_CR3_MASK, val);

	// Clear Status register
	reg_rmw(&usart->regs->SR, USART_SR_MASK, 0);

	// Baud rate register
	usart_set_baud(usart, cfg->baud);

	// Guard time and prescaler register
	reg_rmw(&usart->regs->GTPR, USART_GTPR_MASK, 0);

	return 0;
}

//-----------------------------------------------------------------------------
