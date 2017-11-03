//-----------------------------------------------------------------------------
/*

Random Number Generator Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "stm32f4_soc.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

void rng_isr(struct rng_drv *rng) {
	uint32_t status = rng->regs->SR;
	if ((status & RNG_SR_SEIS) || (status & RNG_SR_CEIS)) {
		// clear the error flag
		rng->regs->SR &= ~(RNG_SR_SEIS | RNG_SR_CEIS);
		// error callback
		if (rng->err_callback) {
			rng->err_callback(rng);
		}
	}
	if (status & RNG_SR_DRDY) {
		uint32_t data = rng->regs->DR;
		if (rng->data_callback) {
			rng->data_callback(rng, data);
		}
	}
}

//-----------------------------------------------------------------------------

void rng_enable(struct rng_drv *rng) {
	rng->regs->CR |= RNG_CR_RNGEN;
}

void rng_disable(struct rng_drv *rng) {
	rng->regs->CR &= ~RNG_CR_RNGEN;
}

//-----------------------------------------------------------------------------

#define RNG_CR_MASK (0xcU)
#define RNG_SR_MASK (0x67U)

int rng_init(struct rng_drv *rng, struct rng_cfg *cfg) {
	memset(rng, 0, sizeof(struct rng_drv));
	rng->regs = (RNG_TypeDef *) RNG_BASE;
	rng->err_callback = cfg->err_callback;
	rng->data_callback = cfg->data_callback;

	// enable the rng module
	RCC->AHB2ENR |= (1 << 6 /*RNGEN*/);

	// control register
	reg_rmw(&rng->regs->CR, RNG_CR_MASK, 0);

	// status register
	reg_rmw(&rng->regs->SR, RNG_SR_MASK, 0);

	return 0;
}

//-----------------------------------------------------------------------------
