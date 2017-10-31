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

// define the non-reserved register bits
#define ADC_CCR_MASK (0xc3ef1fU)
#define ADC_SR_MASK (0x3fU)
#define ADC_CR1_MASK (0x7c0ffffU)
#define ADC_CR2_MASK (0x7f7f0f03U)
#define ADC_SMPR1_MASK (0x7ffffffU)
#define ADC_SMPR2_MASK (0x3fffffffU)
#define ADC_JOFRx_MASK (0xfffU)
#define ADC_HTR_MASK (0xfffU)
#define ADC_LTR_MASK (0xfffU)
#define ADC_SQR1_MASK (0xffffffU)
#define ADC_SQR2_MASK (0x3fffffffU)
#define ADC_SQR3_MASK (0x3fffffffU)
#define ADC_JSQR_MASK (0x3fffffU)

int adc_init(struct adc_drv *adc, struct adc_cfg *cfg) {
	int rc = 0;

	memset(adc, 0, sizeof(struct adc_drv));
	adc->cregs = (ADC_Common_TypeDef *) ADC123_COMMON_BASE;
	adc->regs = (ADC_TypeDef *) cfg->base;

	// enable the adc module
	adc_module_enable(cfg->base);

	adc->cregs->CCR &= ~ADC_CCR_MASK;

	adc->regs->SR &= ~ADC_SR_MASK;
	adc->regs->CR1 &= ~ADC_CR1_MASK;
	adc->regs->CR2 &= ~ADC_CR2_MASK;
	adc->regs->SMPR1 &= ~ADC_SMPR1_MASK;
	adc->regs->SMPR2 &= ~ADC_SMPR2_MASK;
	adc->regs->JOFR1 &= ~ADC_JOFRx_MASK;
	adc->regs->JOFR2 &= ~ADC_JOFRx_MASK;
	adc->regs->JOFR3 &= ~ADC_JOFRx_MASK;
	adc->regs->JOFR4 &= ~ADC_JOFRx_MASK;
	adc->regs->HTR &= ~ADC_HTR_MASK;
	adc->regs->LTR &= ~ADC_LTR_MASK;
	adc->regs->SQR1 &= ~ADC_SQR1_MASK;
	adc->regs->SQR2 &= ~ADC_SQR2_MASK;
	adc->regs->SQR3 &= ~ADC_SQR3_MASK;
	adc->regs->JSQR &= ~ADC_JSQR_MASK;

//exit:
	return rc;
}

//-----------------------------------------------------------------------------
