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

struct rng_drv {
	RNG_TypeDef *regs;
	void (*err_callback) (struct rng_drv * rng);	// errors callback
	void (*data_callback) (struct rng_drv * rng, uint32_t data);	// data callback
};

struct rng_cfg {
	void (*err_callback) (struct rng_drv * rng);	// errors callback
	void (*data_callback) (struct rng_drv * rng, uint32_t data);	// data callback
};

void rng_enable(struct rng_drv *rng);
void rng_disable(struct rng_drv *rng);
int rng_init(struct rng_drv *drv, struct rng_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// RNG_H

//-----------------------------------------------------------------------------
