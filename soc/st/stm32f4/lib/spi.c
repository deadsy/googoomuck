//-----------------------------------------------------------------------------
/*

SPI Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// enable the clock to the spi module
static void spi_enable(uint32_t base) {
	if (base == SPI1_BASE) {
		RCC->APB2ENR |= (1 << 12 /*SPI1 clock enable */ );
	} else if (base == SPI2_BASE) {
		RCC->APB1ENR |= (1 << 14 /*SPI2 clock enable */ );
	} else if (base == SPI3_BASE) {
		RCC->APB1ENR |= (1 << 15 /*SPI3 clock enable */ );
	}
}

//-----------------------------------------------------------------------------

int spi_init(struct spi_drv *spi, struct spi_cfg *cfg) {
	memset(spi, 0, sizeof(struct spi_drv));

	spi->regs = (SPI_TypeDef *) cfg->base;

	// enable the spi module
	spi_enable(cfg->base);

	//spi->regs->CR1;
	//spi->regs->CR2;
	//spi->regs->SR;
	//spi->regs->DR;
	//spi->regs->CRCPR;
	//spi->regs->RXCRCR;
	//spi->regs->TXCRCR;
	//spi->regs->I2SCFGR;
	//spi->regs->I2SPR;

	return 0;
}

//-----------------------------------------------------------------------------
