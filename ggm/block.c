//-----------------------------------------------------------------------------
/*

Block Operations

*/
//-----------------------------------------------------------------------------

#include "ggm.h"
#include "led.h"

//-----------------------------------------------------------------------------
// benchmark

#define BENCHMARK_N 128

void block_benchmark(void) {
	float buf0[BENCHMARK_N];
	//float buf1[BENCHMARK_N];
	float k = 0.1f;
	disable_irq();
	while (1) {
		led_on(LED_AMBER);	// portd,13
		block_mul_k(buf0, k, BENCHMARK_N);
		led_off(LED_AMBER);
	}
}

//-----------------------------------------------------------------------------

#if 0
// multiply two buffers (10uS for n=128)
void block_mul(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] *= buf[i];
	}
}
#endif

// multiply two buffers (6.4uS for n=128)
void block_mul(float *out, float *buf, size_t n) {
	// unroll x4
	float a0, a1, a2, a3;
	float b0, b1, b2, b3;
	float c0, c1, c2, c3;
	while (n > 0) {
		a0 = out[0];
		b0 = buf[0];
		a1 = out[1];
		b1 = buf[1];
		c0 = a0 * b0;
		a2 = out[2];
		b2 = buf[2];
		c1 = a1 * b1;
		a3 = out[3];
		out[0] = c0;
		b3 = buf[3];
		c2 = a2 * b2;
		out[1] = c1;
		c3 = a3 * b3;
		out[2] = c2;
		out[3] = c3;
		out += 4;
		buf += 4;
		n -= 4;
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
	float a0, a1, a2, a3;
	while (n > 0) {
		a0 = out[0];
		a1 = out[1];
		a0 *= k;
		a2 = out[2];
		a1 *= k;
		a3 = out[3];
		a2 *= k;
		a3 *= k;
		out[0] = a0;
		out[1] = a1;
		out[2] = a2;
		out[3] = a3;
		out += 4;
		n -= 4;
	}
}

//-----------------------------------------------------------------------------

// add two buffers (2.31 uS for n = 128)
void block_add(float *out, float *buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] += buf[i];
	}
}

//-----------------------------------------------------------------------------

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
