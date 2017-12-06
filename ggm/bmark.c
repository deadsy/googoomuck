//-----------------------------------------------------------------------------
/*

Benchmarking Functions

*/
//-----------------------------------------------------------------------------

#include "ggm.h"
#include "led.h"

//-----------------------------------------------------------------------------

#define BENCHMARK_N 128

//-----------------------------------------------------------------------------

void pow_benchmark(void) {
	disable_irq();
	int i = 0;
	while (1) {
		led_on(LED_AMBER);	// portd,13
		pow2((float)(i) / 1024.f);
		led_off(LED_AMBER);
		i += 1;
	}
}

void block_benchmark(void) {
	float buf0[BENCHMARK_N];
	float buf1[BENCHMARK_N];
	//float k = 0.1f;
	disable_irq();
	while (1) {
		led_on(LED_AMBER);	// portd,13
		block_copy(buf0, buf1, BENCHMARK_N);
		//block_add_k(buf0, k, BENCHMARK_N);
		led_off(LED_AMBER);
	}
}

//-----------------------------------------------------------------------------
