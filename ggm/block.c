//-----------------------------------------------------------------------------
/*

Block Operations

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

// multiply two buffers
void block_mul(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] *= buf[i];
	}
}

// multiply a block by a scalar
void block_mul_k(float *out, float k, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] *= k;
	}
}

// add two buffers
void block_add(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] += buf[i];
	}
}

// add a scalar to a buffer
void block_add_k(float *out, float k, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] += k;
	}
}

// copy a block
void block_copy(float *dst, float *src, size_t n) {
	for (size_t i = 0; i < n; i++) {
		dst[i] = src[i];
	}
}

//-----------------------------------------------------------------------------
