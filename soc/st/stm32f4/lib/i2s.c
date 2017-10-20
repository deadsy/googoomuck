//-----------------------------------------------------------------------------
/*

I2S Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>
#include <assert.h>

#define DEBUG

#include "stm32f4_soc.h"
#include "logging.h"
#include "utils.h"

//-----------------------------------------------------------------------------

// setup the i2s clock
int i2s_clock(uint32_t fs) {
	RCC_PeriphCLKInitTypeDef rccclkinit;
	int idx = 0;
	static const uint32_t cfg[][3] = {
		// Fs, PLLI2SN, PLLI2SR
		{8000, 256, 5},	// clk = 51200000
		{11025, 429, 4},	// clk = 107250000
		{16000, 213, 4},	// clk = 53250000
		{22050, 429, 4},	// clk = 107250000
		{32000, 426, 4},	// clk = 106500000
		{44100, 271, 6},	// clk = 45166666
		{48000, 258, 3},	// clk = 86000000
		{96000, 344, 1},	// clk = 344000000
		//{96000, 290, 2},      // clk = 145000000
		{0, 258, 3},	// clk = 86000000 (end of list, default)
	};
	// lookup the table
	while (cfg[idx][0]) {
		if (cfg[idx][0] == fs) {
			break;
		}
		idx++;
	}
	// Enable PLLI2S clock
	// F_PLLI2S_clock_input = HSE_VALUE
	// F_VCO_clock = F_PLLI2S_clock_input × (PLLI2SN / PLLM)
	// F_PLLI2S_clock_output = F_VCO_clock / PLLI2SR
	// also: F_PLLI2S_clock_output = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_I2S)
	HAL_RCCEx_GetPeriphCLKConfig(&rccclkinit);
	rccclkinit.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	rccclkinit.PLLI2S.PLLI2SN = cfg[idx][1];
	rccclkinit.PLLI2S.PLLI2SR = cfg[idx][2];
	return (int)HAL_RCCEx_PeriphCLKConfig(&rccclkinit);
}

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

#if 0

static void i2s_enable(struct i2s_drv *i2s) {
	i2s->base->I2SCFGR |= (1 << 10);	// I2SE
}

static void i2s_disable(struct i2s_drv *i2s) {
	i2s->base->I2SCFGR &= ~(1 << 10);	// I2SE
}

#endif

//-----------------------------------------------------------------------------

#define I2SCFGR_MASK ((0x1f << 7) | 0x3f)
#define I2SPR_MASK (0x3ff)

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg) {
	uint32_t val;
	uint32_t clk, odd, div;
	memset(i2s, 0, sizeof(struct i2s_drv));
	i2s->cfg = *cfg;
	// TODO support full duplex mode
	assert(i2s->cfg.fdx_mode == 0);
	spi_enable(i2s);
	i2s->base = spi_base(i2s->cfg.idx);
	// clear I2SCFGR
	i2s->base->I2SCFGR &= ~I2SCFGR_MASK;
	// set I2SPR to the reset value
	reg_rmw(&i2s->base->I2SPR, I2SPR_MASK, 2);
	// setup I2SCFGR
	val = (1 << 11 /*I2SMOD */ );
	val |= (i2s->cfg.mode | i2s->cfg.standard | i2s->cfg.clk_polarity | i2s->cfg.data_format);
	reg_rmw(&i2s->base->I2SCFGR, I2SCFGR_MASK, val);
	// setup I2SPR
	clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_I2S);
	DBG("clk %d\r\n", clk);
	//TODO derive values
	odd = 1;
	div = 6;
	reg_rmw(&i2s->base->I2SPR, 0x3ff, (odd << 8) | (div << 0) | i2s->cfg.mclk_output);
	return 0;
}

//-----------------------------------------------------------------------------
