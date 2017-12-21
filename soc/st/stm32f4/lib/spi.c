//-----------------------------------------------------------------------------
/*

SPI Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// enable the clock to the spi module
void spi_enable(uint32_t base) {
	if (base == SPI1_BASE) {
		RCC->APB2ENR |= (1 << 12 /*SPI1 clock enable */ );
	} else if (base == SPI2_BASE) {
		RCC->APB1ENR |= (1 << 14 /*SPI2 clock enable */ );
	} else if (base == SPI3_BASE) {
		RCC->APB1ENR |= (1 << 15 /*SPI3 clock enable */ );
	}
}

//-----------------------------------------------------------------------------

// tx buffer empty
static inline int spi_txe(struct spi_drv *spi) {
	return spi->regs->SR & (1 << 1 /*TXE*/);
}

// rx buffer not empty
static inline int spi_rxne(struct spi_drv *spi) {
	return spi->regs->SR & (1 << 0 /*RXNE*/);
}

// read the spi data register (non-blocking)
int spi_rd(struct spi_drv *spi, uint16_t * data) {
	if (spi_rxne(spi)) {
		*data = spi->regs->DR;
		return 0;
	}
	return -1;
}

// read the spi data register (blocking)
void spi_rd_block(struct spi_drv *spi, uint16_t * data) {
	while (1) {
		if (spi_rd(spi, data) == 0) {
			return;
		}
		udelay(5);
	}
}

// write the spi data register (non-blocking)
int spi_wr(struct spi_drv *spi, uint16_t data) {
	if (spi_txe(spi)) {
		spi->regs->DR = data;
		return 0;
	}
	return -1;
}

// write the spi data register (blocking)
void spi_wr_block(struct spi_drv *spi, uint16_t data) {
	while (1) {
		if (spi_wr(spi, data) == 0) {
			return;
		}
		udelay(5);
	}
}

//-----------------------------------------------------------------------------

int spi_init(struct spi_drv *spi, struct spi_cfg *cfg) {
	uint32_t val;

	memset(spi, 0, sizeof(struct spi_drv));
	spi->cfg = *cfg;
	spi->regs = (SPI_TypeDef *) cfg->base;

	// enable the spi module
	spi_enable(cfg->base);

	// turn off the peripheral for configuration
	reg_clr(&spi->regs->CR1, (1U << 6 /*SPE*/));

	// setup CR1
	val = 0;
	val |= (0 << 15 /*BIDIMODE*/);	// Bidirectional data mode enable
	val |= (0 << 14 /*BIDIOE*/);	// Output enable in bidirectional mode
	val |= (0 << 13 /*CRCEN*/);	// Hardware CRC calculation enable
	val |= (0 << 12 /*CRCNEXT*/);	// CRC transfer next
	val |= (0 << 10 /*RXONLY*/);	// Receive only
	// note: software drives the slave chip select as a normal GPIO
	val |= (1 << 9 /*SSM*/);	// Software slave management
	val |= (1 << 8 /*SSI*/);	// Internal slave select
	val |= spi->cfg.ff;	// Data frame format and MSB/LSB
	val |= spi->cfg.div;	// Baud rate control
	val |= spi->cfg.mode;	// Master selection
	val |= spi->cfg.cpol;	// Clock polarity
	val |= spi->cfg.cpha;	// Clock phase
	reg_rmw(&spi->regs->CR1, CR1_MASK, val);

	// setup CR2
	val = 0;
	val |= (0 << 7 /*TXEIE*/);	// Tx buffer empty interrupt enable
	val |= (0 << 6 /*RXNEIE*/);	// RX buffer not empty interrupt enable
	val |= (0 << 5 /*ERRIE*/);	// Error interrupt enable
	val |= (0 << 4 /*FRF*/);	// Frame format
	// note: set the SSOE bit to stop the SPI immediately giving a MODF error
	val |= (1 << 2 /*SSOE*/);	// SS output enable
	val |= (0 << 1 /*TXDMAEN*/);	// Tx buffer DMA enable
	val |= (0 << 0 /*RXDMAEN*/);	// Rx buffer DMA enable
	reg_rmw(&spi->regs->CR2, CR2_MASK, val);

	// TODO configure CRC operation
	//spi->regs->CRCPR;
	//spi->regs->RXCRCR;
	//spi->regs->TXCRCR;

	// setup I2SCFGR (make sure we are in SPI mode)
	reg_clr(&spi->regs->I2SCFGR, (1 << 11 /*I2SMODE */ ));

	// turn on the peripheral
	reg_set(&spi->regs->CR1, (1U << 6 /*SPE*/));
	return 0;
}

//-----------------------------------------------------------------------------
