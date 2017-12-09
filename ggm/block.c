//-----------------------------------------------------------------------------
/*

Block Operations

Note: Unrolling these loops and breaking up the operations tends to speed them
up because it gives the compiler a chance to pipeline loads and stores.
I haven't tried anything fancy with the order of operations within the loop
because the compiler will re-order them as it sees fit.

The block_mul/add() function seem immune to improvements. They use vldmia/vstmia
and it maybe that other functions could benefit from multiple load/store also.

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

// multiply two buffers (2.31uS for n=128)
void block_mul(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] *= buf[i];
	}
}

//-----------------------------------------------------------------------------

#if 0
// multiply a block by a scalar (2.31uS for n=128)
void block_mul_k(float *out, float k, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] *= k;
	}
}
#endif

// multiply a block by a scalar (0.6uS for n=128)
void block_mul_k(float *out, float k, size_t n) {
	// unroll x4
	while (n > 0) {
		out[0] *= k;
		out[1] *= k;
		out[2] *= k;
		out[3] *= k;
		out += 4;
		n -= 4;
	}
}

//-----------------------------------------------------------------------------

// add two buffers (2.31uS for n=128)
void block_add(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] += buf[i];
	}
}

//-----------------------------------------------------------------------------

#if 0
// add a scalar to a buffer (2.31uS for n=128)
void block_add_k(float *out, float k, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] += k;
	}
}
#endif

// add a scalar to a buffer (0.6uS for n=128)
void block_add_k(float *out, float k, size_t n) {
	// unroll x4
	while (n > 0) {
		out[0] += k;
		out[1] += k;
		out[2] += k;
		out[3] += k;
		out += 4;
		n -= 4;
	}
}

//-----------------------------------------------------------------------------

#if 0
// copy a block (2.31uS for n=128)
void block_copy(float *dst, float *src, size_t n) {
	for (size_t i = 0; i < n; i++) {
		dst[i] = src[i];
	}
}
#endif

// copy a block (0.6uS for n=128)
void block_copy(float *dst, const float *src, size_t n) {
	// unroll x4
	while (n > 0) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
		src += 4;
		dst += 4;
		n -= 4;
	}
}

// copy a block and multiply by k
void block_copy_mul_k(float *dst, const float *src, float k, size_t n) {
	// unroll x4
	while (n > 0) {
		dst[0] = src[0] * k;
		dst[1] = src[1] * k;
		dst[2] = src[2] * k;
		dst[3] = src[3] * k;
		src += 4;
		dst += 4;
		n -= 4;
	}
}

//-----------------------------------------------------------------------------
