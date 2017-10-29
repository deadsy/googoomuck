//-----------------------------------------------------------------------------
/*

ADC Driver

*/
//-----------------------------------------------------------------------------

#ifndef ADC_H
#define ADC_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

struct adc_cfg {
	uint32_t base;		// base address
};

struct adc_drv {
	ADC_Common_TypeDef *cregs;	// common registers
	ADC_TypeDef *regs;	// specific adc registers
};

int adc_init(struct adc_drv *adc, struct adc_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// ADC_H

//-----------------------------------------------------------------------------
