//-----------------------------------------------------------------------------
/*

GPIO Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#ifndef GPIO_H
#define GPIO_H

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

//-----------------------------------------------------------------------------
// port numbers

#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5
#define PORTG 6
#define PORTH 7
#define PORTI 8

// pin modes
#define GPIO_MODER_IN 0		// Input mode (reset state)
#define GPIO_MODER_OUT 1	// General purpose output mode
#define GPIO_MODER_AF 2		// Alternate function mode
#define GPIO_MODER_AN 3		// Analog mode

// output type
#define GPIO_OTYPER_PP 0	// Output push-pull (reset state)
#define GPIO_OTYPER_OD 1	// Output open-drain

// output speed
#define GPIO_OSPEEDR_LO 0	// Low speed
#define GPIO_OSPEEDR_MED 1	// Medium speed
#define GPIO_OSPEEDR_FAST 2	// Fast speed
#define GPIO_OSPEEDR_HI 3	// High speed

// pull up/down
#define GPIO_PUPD_NONE 0	// No pull-up, pull-down
#define GPIO_PUPD_PU 1		// Pull-up
#define GPIO_PUPD_PD 2		// Pull-down

// alternate functions
#define GPIO_AF0 0
#define GPIO_AF1 1
#define GPIO_AF2 2
#define GPIO_AF3 3
#define GPIO_AF4 4
#define GPIO_AF5 5
#define GPIO_AF6 6
#define GPIO_AF7 7
#define GPIO_AF8 8
#define GPIO_AF9 9
#define GPIO_AF10 10
#define GPIO_AF11 11
#define GPIO_AF12 12
#define GPIO_AF13 13
#define GPIO_AF14 14
#define GPIO_AF15 15

//-----------------------------------------------------------------------------
// gpio macros

#define GPIO_NUM(port, pin) ((port << 4) | (pin))
#define GPIO_PORT(n) (n >> 4)
#define GPIO_PIN(n) (n & 0xf)
#define GPIO_BIT(n) (1 << GPIO_PIN(n))
#define GPIO_BASE(n) ((GPIO_TypeDef *)(GPIOA_BASE + (GPIO_PORT(n) * 0x400)))

//-----------------------------------------------------------------------------

struct gpio_info {
	int num;		// gpio number
	int mode;		// pin mode
	int type;		// output type
	int speed;		// outputpin speed
	int pupd;		// pull up/down
	int af;			// alternate function
	int init;		// initial pin value
};

//-----------------------------------------------------------------------------
// generic api functions

static inline void gpio_clr(int n) {
	GPIO_BASE(n)->BSRR = 1 << (GPIO_PIN(n) + 16);
}

static inline void gpio_set(int n) {
	GPIO_BASE(n)->BSRR = 1 << GPIO_PIN(n);
}

static inline void gpio_toggle(int n) {
	GPIO_BASE(n)->ODR ^= GPIO_BIT(n);
}

static inline int gpio_rd(int n) {
	return (GPIO_BASE(n)->IDR >> GPIO_PIN(n)) & 1;
}

static inline int gpio_rd_inv(int n) {
	return (~(GPIO_BASE(n)->IDR) >> GPIO_PIN(n)) & 1;
}

static inline void gpio_dirn_in(int n) {
	// set the MODER bits to 0 (input) for the pin
	GPIO_BASE(n)->MODER &= ~(3 << (GPIO_PIN(n) * 2));
}

static inline void gpio_dirn_out(int n) {
	// set the MODER bits to 1 (ouput) for the pin
	uint32_t mode = GPIO_BASE(n)->MODER;
	mode &= ~(3 << (GPIO_PIN(n) * 2));
	mode |= (1 << (GPIO_PIN(n) * 2));
	GPIO_BASE(n)->MODER = mode;
}

int gpio_init(const struct gpio_info *info, size_t n);

//-----------------------------------------------------------------------------

#endif				// GPIO_H

//-----------------------------------------------------------------------------
