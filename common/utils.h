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
// Q format to float conversions

static inline float q31_to_float(int x) {
	return (float)x / (float)(1 << 31);
}

//-----------------------------------------------------------------------------
// clamp floating point values

// clamp x between a and b
static inline float clampf(float x, float a, float b) {
	if (x < a) {
		return a;
	}
	if (x > b) {
		return b;
	}
	return x;
}

// clamp x to >= a
static inline float clampf_lo(float x, float a) {
	return (x < a) ? a : x;
}

// clamp x to <= a
static inline float clampf_hi(float x, float a) {
	return (x > a) ? a : x;
}

//-----------------------------------------------------------------------------

// linear mapping map x = 0..1 to y = a..b
static inline float mapf(float x, float a, float b) {
	return ((b - a) * x) + a;
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
