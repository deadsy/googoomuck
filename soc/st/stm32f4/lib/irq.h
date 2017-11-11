//-----------------------------------------------------------------------------
/*

IRQ Control for the Cortex-M

*/
//-----------------------------------------------------------------------------

#ifndef IRQ_H
#define IRQ_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

static inline uint32_t disable_irq(void) {
	uint32_t x;
	__asm__ volatile ("mrs %0, primask":"=r" (x));
	__asm__ volatile ("cpsid i":::"memory");
	return x;
}

static inline void restore_irq(uint32_t x) {
	__asm__ volatile ("msr primask, %0"::"r" (x):"memory");
}

//-----------------------------------------------------------------------------

#endif				// IRQ_H

//-----------------------------------------------------------------------------
