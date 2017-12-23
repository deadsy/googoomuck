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
#if defined(SPI_DRIVER_HW)

// tx buffer empty
static inline int spi_tx_empty(struct spi_drv *spi) {
	return spi->regs->SR & (1 << 1 /*TXE*/);
}

// rx buffer not empty
static inline int spi_rx_not_empty(struct spi_drv *spi) {
	return spi->regs->SR & (1 << 0 /*RXNE*/);
}

int spi_txrx(struct spi_drv *spi, uint32_t tx, uint32_t * rx) {
	while (!spi_tx_empty(spi)) {
		udelay(5);
	}
	spi->regs->DR = tx;
	uint16_t data = spi->regs->DR;
	if (rx) {
		*rx = data;
	}
	return 0;
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
	val |= spi->cfg.bits;	// 8/16 bits per frame
	val |= spi->cfg.lsb;	// msb/lsb first
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
#elif defined(SPI_DRIVER_BITBANG)

// set or clear the MOSI line
static inline void spi_mosi(struct spi_drv *spi, uint32_t x) {
	if (x) {
		gpio_set(spi->cfg.mosi);
	} else {
		gpio_clr(spi->cfg.mosi);
	}
}

// pulse the clock line and read the MISO line
static int spi_clk_pulse(struct spi_drv *spi) {
	int rc = 0;
	// 1st clock edge
	gpio_toggle(spi->cfg.clk);
	udelay(spi->cfg.delay);
	if (!spi->cfg.cpha) {
		rc = gpio_rd(spi->cfg.miso);
	}
	// 2nd clock edge
	gpio_toggle(spi->cfg.clk);
	udelay(spi->cfg.delay);
	if (spi->cfg.cpha) {
		rc = gpio_rd(spi->cfg.miso);
	}
	return rc;
}

//-----------------------------------------------------------------------------

// Tx and Rx N bits.
static uint32_t spi_txrx_n(struct spi_drv *spi, uint32_t tx, int n) {
	uint32_t rx = 0;
	n -= 1;
	if (spi->cfg.lsb) {
		// least significant bit first
		uint32_t mask = 1;
		for (int i = 0; i <= n; i++) {
			spi_mosi(spi, tx & mask);
			rx >>= 1;
			rx |= spi_clk_pulse(spi) << n;
			mask <<= 1;
		}
	} else {
		// most significant bit first
		uint32_t mask = 1U << n;
		for (int i = 0; i <= n; i++) {
			spi_mosi(spi, tx & mask);
			rx <<= 1;
			rx |= spi_clk_pulse(spi);
			mask >>= 1;
		}
	}
	return rx;
}

//-----------------------------------------------------------------------------
// Tx

// Tx 8 bits
int spi_tx8(struct spi_drv *spi, uint8_t data) {
	spi_txrx_n(spi, data, 8);
	return 0;
}

// Tx an 8 bit buffer
int spi_txbuf8(struct spi_drv *spi, const uint8_t * buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		spi_txrx_n(spi, buf[i], 8);
	}
	return 0;
}

// Tx 16 bits
int spi_tx16(struct spi_drv *spi, uint16_t data) {
	spi_txrx_n(spi, data, 16);
	return 0;
}

// Tx a 16 bit buffer
int spi_txbuf16(struct spi_drv *spi, const uint16_t * buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		spi_txrx_n(spi, buf[i], 16);
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Rx

// Rx an 8 bit buffer
int spi_rxbuf8(struct spi_drv *spi, uint8_t * buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		buf[i] = spi_txrx_n(spi, 0, 8);
	}
	return 0;
}

//-----------------------------------------------------------------------------

int spi_init(struct spi_drv *spi, struct spi_cfg *cfg) {
	memset(spi, 0, sizeof(struct spi_drv));
	spi->cfg = *cfg;
	// set the clock line per polarity
	if (spi->cfg.cpol) {
		gpio_set(spi->cfg.clk);
	} else {
		gpio_clr(spi->cfg.clk);
	}
	return 0;
}

//-----------------------------------------------------------------------------

#else
#error "what kind of SPI driver are we building?"
#endif

//-----------------------------------------------------------------------------
