//-----------------------------------------------------------------------------
/*

Random Number Generator Driver

*/
//-----------------------------------------------------------------------------

#ifndef RNG_H
#define RNG_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

#define RNG_MODE_POLLED (0U << 3/*IE*/)
#define RNG_MODE_INTERRUPT (1U << 3/*IE*/)

struct rng_drv {
	RNG_TypeDef *regs;
	void (*err_callback) (struct rng_drv * rng, uint32_t errors);	// errors callback
	void (*data_callback) (struct rng_drv * rng, uint32_t data);	// data callback
};

struct rng_cfg {
	uint32_t mode;
	void (*err_callback) (struct rng_drv * rng, uint32_t errors);	// errors callback
	void (*data_callback) (struct rng_drv * rng, uint32_t data);	// data callback
};

static inline void rng_enable(struct rng_drv *rng) {
	rng->regs->CR |= RNG_CR_RNGEN;
}

static inline void rng_disable(struct rng_drv *rng) {
	rng->regs->CR &= ~RNG_CR_RNGEN;
}

int rng_init(struct rng_drv *drv, struct rng_cfg *cfg);
int rng_rd(struct rng_drv *rng, int block, uint32_t * data);
void rng_isr(struct rng_drv *rng);

//-----------------------------------------------------------------------------

#endif				// RNG_H

//-----------------------------------------------------------------------------
