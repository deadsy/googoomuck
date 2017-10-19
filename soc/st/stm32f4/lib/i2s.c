//-----------------------------------------------------------------------------
/*

I2S Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>
#include <assert.h>

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
	uint32_t val;

	memset(i2s, 0, sizeof(struct i2s_drv));
	i2s->cfg = *cfg;

	spi_enable(i2s);

	i2s->base = spi_base(i2s->cfg.idx);

	// clear I2SCFGR - preserve reserved bits
	i2s->base->I2SCFGR &= (15 << 12) | (1 << 6);
	// set I2SPR to the reset value
	i2s->base->I2SPR = 2;

	// setup I2SCFGR
	val = i2s->base->I2SCFGR;
	val |= (1 << 11 /*I2SMOD */ );	// I2S is enabled
	val |= (0 << 10 /*I2SE */ );	// I2S peripheral is disabled
	val |= (i2s->cfg.mode | i2s->cfg.standard | i2s->cfg.clk_polarity | i2s->cfg.data_format);
	i2s->base->I2SCFGR = val;

	// TODO support full duplex mode
	assert(i2s->cfg.fdx_mode == 0);

#if 0

	haudio_in_i2s.Init.AudioFreq = 4 * AudioFreq;
	haudio_in_i2s.Init.ClockSource = I2S_CLOCK_PLL;
	haudio_in_i2s.Init.CPOL = I2S_CPOL_HIGH;
	haudio_in_i2s.Init.DataFormat = I2S_DATAFORMAT_16B;
	haudio_in_i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	haudio_in_i2s.Init.Mode = I2S_MODE_MASTER_RX;
	haudio_in_i2s.Init.Standard = I2S_STANDARD_LSB;

#endif
	return 0;
}

//-----------------------------------------------------------------------------
