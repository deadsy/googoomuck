//-----------------------------------------------------------------------------
/*

ADC Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// enable the clock to the adc module
static void adc_module_enable(uint32_t base) {
	if (base == ADC1_BASE) {
		RCC->APB2ENR |= (1 << 8 /*ADC1EN */ );
	} else if (base == ADC2_BASE) {
		RCC->APB2ENR |= (1 << 9 /*ADC2EN */ );
	} else if (base == ADC3_BASE) {
		RCC->APB2ENR |= (1 << 10 /*ADC3EN */ );
	}
}

//-----------------------------------------------------------------------------

int adc_init(struct adc_drv *adc, struct adc_cfg *cfg) {
	int rc = 0;

	memset(adc, 0, sizeof(struct adc_drv));
	adc->cregs = (ADC_Common_TypeDef *) ADC123_COMMON_BASE;
	adc->regs = (ADC_TypeDef *) cfg->base;

	// enable the adc module
	adc_module_enable(cfg->base);

	adc->cregs->CCR = 0;

	adc->regs->SR = 0;
	adc->regs->CR1 = 0;
	adc->regs->CR2 = 0;
	adc->regs->SMPR1 = 0;
	adc->regs->SMPR2 = 0;
	adc->regs->JOFR1 = 0;
	adc->regs->JOFR2 = 0;
	adc->regs->JOFR3 = 0;
	adc->regs->JOFR4 = 0;
	adc->regs->HTR = 0;
	adc->regs->LTR = 0;
	adc->regs->SQR1 = 0;
	adc->regs->SQR2 = 0;
	adc->regs->SQR3 = 0;
	adc->regs->JSQR = 0;

//exit:
	return rc;
}

//-----------------------------------------------------------------------------
