//-----------------------------------------------------------------------------
/*

Common Utility Functions/Macros

*/
//-----------------------------------------------------------------------------

#ifndef UTILS_H
#define UTILS_H

//-----------------------------------------------------------------------------

#define ALIGN(x) __attribute__ ((aligned (x)))

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
