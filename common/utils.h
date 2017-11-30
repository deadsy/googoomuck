//-----------------------------------------------------------------------------
/*

Common Utility Functions/Macros

*/
//-----------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

//-----------------------------------------------------------------------------

#include <stdlib.h>

//-----------------------------------------------------------------------------

#define ALIGN(x) __attribute__ ((aligned (x)))

//-----------------------------------------------------------------------------

static inline float q31_to_f(int x) {
	return (float)x *(1.f / (float)(1 << 31));
}

//-----------------------------------------------------------------------------

// clamp x between a and b
static inline float clamp(float x, float a, float b) {
	x = (x < a) ? a : x;
	x = (x > b) ? b : x;
	return x;
}

//-----------------------------------------------------------------------------

// return a float between a..b
static inline float randf_range(float a, float b) {
	return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

//-----------------------------------------------------------------------------

static inline void reg_rmw(volatile uint32_t * reg, uint32_t mask, uint32_t val) {
	uint32_t x = *reg;
	x &= ~mask;
	x |= (val & mask);
	*reg = x;
}

static inline void reg_set(volatile uint32_t * reg, uint32_t bits) {
	reg_rmw(reg, bits, 0xffffffff);
}

static inline void reg_clr(volatile uint32_t * reg, uint32_t bits) {
	reg_rmw(reg, bits, 0);
}

//-----------------------------------------------------------------------------

#endif				// UTILS_H

//-----------------------------------------------------------------------------
