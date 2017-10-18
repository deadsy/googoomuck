//-----------------------------------------------------------------------------
/*

Polled Delays

*/
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

#define TICK_CLK 168		// systick count down clock in MHz

static uint64_t get_current_usecs(void) {
	return (HAL_GetTick() * 1000) + 1000 - (SysTick->VAL / TICK_CLK);
}

//-----------------------------------------------------------------------------

void mdelay(unsigned long msecs) {
	HAL_Delay(msecs);
}

void udelay(unsigned long usecs) {
	uint64_t timeout = get_current_usecs() + usecs;
	while (get_current_usecs() < timeout) ;
}

//-----------------------------------------------------------------------------
