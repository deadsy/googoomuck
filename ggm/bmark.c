//-----------------------------------------------------------------------------
/*

Benchmarking Functions

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

#define BENCHMARK_N 128

//-----------------------------------------------------------------------------

void pow_benchmark(void) {
	disable_irq();
	int i = 0;
	while (1) {
		gpio_set(IO_LED_AMBER);	// PORTD, 13
		pow2((float)(i) / 1024.f);
		gpio_clr(IO_LED_AMBER);
		i += 1;
	}
}

void block_benchmark(void) {
	float buf0[BENCHMARK_N];
	float buf1[BENCHMARK_N];
	//float k = 0.1f;
	disable_irq();
	while (1) {
		gpio_set(IO_LED_AMBER);	// PORTD, 13
		block_copy(buf0, buf1, BENCHMARK_N);
		//block_add_k(buf0, k, BENCHMARK_N);
		gpio_clr(IO_LED_AMBER);
	}
}

//-----------------------------------------------------------------------------
