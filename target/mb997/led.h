//-----------------------------------------------------------------------------
/*

LED Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#ifndef LED_H
#define LED_H

//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

// leds
#define LED_GREEN       GPIO_NUM(PORTD, 12)
#define LED_AMBER       GPIO_NUM(PORTD, 13)
#define LED_RED         GPIO_NUM(PORTD, 14)
#define LED_BLUE        GPIO_NUM(PORTD, 15)

static inline void led_on(int x) {
	gpio_set(x);
}

static inline void led_off(int x) {
	gpio_clr(x);
}

//-----------------------------------------------------------------------------

#endif				// LED_H

//-----------------------------------------------------------------------------
