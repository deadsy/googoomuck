//-----------------------------------------------------------------------------
/*

I2S Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

// return a pointer to the spi registers (1,2,3)
static inline SPI_TypeDef *spi_base(int n) {
	static const uint32_t base[] = { 0, SPI1_BASE, SPI2_BASE, SPI3_BASE };
	return (SPI_TypeDef *) base[n];
}

// return a point to the i2sext registers (2,3)
static inline SPI_TypeDef *i2sext_base(int n) {
	static const uint32_t base[] = { 0, 0, I2S2ext_BASE, I2S3ext_BASE };
	return (SPI_TypeDef *) base[n];
}

//-----------------------------------------------------------------------------

// enable the clock to the spi module
static void spi_enable(struct i2s_drv *i2s) {
	int idx = i2s->cfg.idx;
	if (idx == 1) {
		RCC->APB2ENR |= (1 << 12 /*SPI1 clock enable */ );
	} else if (idx == 2) {
		RCC->APB1ENR |= (1 << 14 /*SPI2 clock enable */ );
	} else if (idx == 3) {
		RCC->APB1ENR |= (1 << 15 /*SPI3 clock enable */ );
	}
}

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg) {

	memset(i2s, 0, sizeof(struct i2s_drv));
	i2s->cfg = *cfg;

	spi_enable(i2s);

	i2s->base = spi_base(i2s->cfg.idx);
	i2s->base->I2SCFGR &= ~(SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CKPOL | SPI_I2SCFGR_I2SSTD | SPI_I2SCFGR_PCMSYNC | SPI_I2SCFGR_I2SCFG | SPI_I2SCFGR_I2SE | SPI_I2SCFGR_I2SMOD);
	i2s->base->I2SPR = 2U;

	return 0;
}

//-----------------------------------------------------------------------------
