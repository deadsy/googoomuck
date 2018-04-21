//-----------------------------------------------------------------------------
/*

Serial Audio Interface (SAI) Driver

*/
//-----------------------------------------------------------------------------

#ifndef SAI_H
#define SAI_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------
// SAI Driver API

struct sai_cfg {
	uint32_t base;		// base address of sai peripheral
};

struct sai_drv {
	struct sai_cfg cfg;	// configuration values
	SAI_TypeDef *global;
	SAI_Block_TypeDef *blocka;
	SAI_Block_TypeDef *blockb;
};

int sai_init(struct sai_drv *sai, struct sai_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// SAI_H

//-----------------------------------------------------------------------------
