//-----------------------------------------------------------------------------
/*

I2S Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg, SPI_TypeDef * base) {

	memset(i2s, 0, sizeof(struct i2s_drv));
	memcpy(&i2s->cfg, cfg, sizeof(struct i2s_cfg));
	i2s->base = base;

	i2s->base->I2SCFGR &= ~(SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CKPOL | SPI_I2SCFGR_I2SSTD | SPI_I2SCFGR_PCMSYNC | SPI_I2SCFGR_I2SCFG | SPI_I2SCFGR_I2SE | SPI_I2SCFGR_I2SMOD);
	i2s->base->I2SPR = 2U;

	return 0;
}

//-----------------------------------------------------------------------------
