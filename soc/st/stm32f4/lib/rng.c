//-----------------------------------------------------------------------------
/*

Random Number Generator Driver

Notes:
On the STM32F407 running at 168 MHz, 1024*1024 words are generated in 918ms.
ie: 0.88us/word, or 147 cycles/word.

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
			rng->err_callback(rng, status);
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

// read a random number if it is available
static int rng_rd_non_blocking(struct rng_drv *rng, uint32_t * data) {
	if (rng->regs->SR & RNG_SR_DRDY) {
		*data = rng->regs->DR;
		return 0;
	}
	return -1;
}

#define RNG_TIMEOUT 2U

// read a random number, blocking or non-blocking
int rng_rd(struct rng_drv *rng, int block, uint32_t * data) {
	*data = 0;
	if (block) {
		uint32_t t = HAL_GetTick();
		do {
			if (rng->regs->SR & RNG_SR_DRDY) {
				break;
			}
		} while ((HAL_GetTick() - t) < RNG_TIMEOUT);
	}
	return rng_rd_non_blocking(rng, data);
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
	reg_rmw(&rng->regs->CR, RNG_CR_MASK, cfg->mode);

	// status register
	reg_rmw(&rng->regs->SR, RNG_SR_MASK, 0);

	return 0;
}

//-----------------------------------------------------------------------------
