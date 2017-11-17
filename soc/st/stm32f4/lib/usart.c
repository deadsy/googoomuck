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

#define INC_MOD(x, s) (((x) + 1) & ((s) - 1))

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

// return the irq used for this usart module
static int usart_irq(uint32_t base) {
	if (base == USART1_BASE) {
		return USART1_IRQn;
	} else if (base == USART2_BASE) {
		return USART2_IRQn;
	} else if (base == USART3_BASE) {
		return USART3_IRQn;
	} else if (base == UART4_BASE) {
		return UART4_IRQn;
	} else if (base == UART5_BASE) {
		return UART5_IRQn;
	} else if (base == USART6_BASE) {
		return USART6_IRQn;
	}
	return 0;
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
// stdio functions

void usart_putc(struct usart_drv *usart, char c) {
	int tx_wr_inc = INC_MOD(usart->tx_wr, TXBUF_SIZE);
	// wait for space
	while (tx_wr_inc == usart->tx_rd) ;
	// put the character in the tx buffer
	NVIC_DisableIRQ(usart->irq);
	usart->txbuf[usart->tx_wr] = c;
	usart->tx_wr = tx_wr_inc;
	NVIC_EnableIRQ(usart->irq);
	// enable the tx empty interrupt
	usart->regs->CR1 |= USART_CR1_TXEIE;
}

void usart_flush(struct usart_drv *usart) {
	while (usart->tx_wr != usart->tx_rd) ;
}

// return non-zero if we have rx data
int usart_tstc(struct usart_drv *usart) {
	return usart->rx_rd != usart->rx_wr;
}

char usart_getc(struct usart_drv *usart) {
	// wait for a character
	while (usart_tstc(usart) == 0) ;
	NVIC_DisableIRQ(usart->irq);
	char c = usart->rxbuf[usart->rx_rd];
	usart->rx_rd = INC_MOD(usart->rx_rd, RXBUF_SIZE);
	NVIC_EnableIRQ(usart->irq);
	return c;
}

//-----------------------------------------------------------------------------

// read serial data into a buffer, return the number of bytes read
size_t usart_rxbuf(struct usart_drv * usart, uint8_t * buf, size_t n) {
	size_t i = 0;
	if ((usart->rx_rd == usart->rx_wr) || (n == 0)) {
		return 0;
	}
	NVIC_DisableIRQ(usart->irq);
	while (usart->rx_rd != usart->rx_wr) {
		buf[i++] = usart->rxbuf[usart->rx_rd];
		usart->rx_rd = INC_MOD(usart->rx_rd, RXBUF_SIZE);
		if (i == n) {
			break;
		}
	}
	NVIC_EnableIRQ(usart->irq);
	return i;
}

//-----------------------------------------------------------------------------

void usart_isr(struct usart_drv *usart) {
	uint32_t status = usart->regs->SR;

	// check for rx errors
	if (status & (USART_SR_ORE | USART_SR_PE | USART_SR_FE | USART_SR_NE)) {
		usart->rx_errors++;
	}
	// receive
	if (status & USART_SR_RXNE) {
		uint8_t c = usart->regs->DR;
		int rx_wr_inc = INC_MOD(usart->rx_wr, RXBUF_SIZE);
		if (rx_wr_inc != usart->rx_rd) {
			usart->rxbuf[usart->rx_wr] = c;
			usart->rx_wr = rx_wr_inc;
		} else {
			// rx buffer overflow
			usart->rx_errors++;
		}
	}
	// transmit
	if (status & USART_SR_TXE) {
		if (usart->tx_rd != usart->tx_wr) {
			usart->regs->DR = usart->txbuf[usart->tx_rd];
			usart->tx_rd = INC_MOD(usart->tx_rd, TXBUF_SIZE);
		} else {
			// no more tx data, disable the tx empty interrupt
			usart->regs->CR1 &= ~USART_CR1_TXEIE;
		}
	}

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
	usart->irq = usart_irq(cfg->base);

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
	val |= (1 << 5 /*RXNEIE*/);	// RXNE interrupt enable
	val |= (0 << 4 /*IDLEIE*/);	// IDLE interrupt enable
	val |= (1 << 3 /*TE*/);	// Transmitter enable
	val |= (1 << 2 /*RE*/);	// Receiver enable
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

	// enable the uart
	usart->regs->CR1 |= USART_CR1_UE;

	return 0;
}

//-----------------------------------------------------------------------------
