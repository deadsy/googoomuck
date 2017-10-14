//-----------------------------------------------------------------------------
/*

GPIO Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#ifndef GPIO_H
#define GPIO_H

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

//-----------------------------------------------------------------------------
// gpio macros

#define GPIO_NUM(port, pin) ((port << 4) | (pin))
#define GPIO_PORT(n) (n >> 4)
#define GPIO_PIN(n) (n & 0xf)
#define GPIO_BIT(n) (1 << GPIO_PIN(n))
#define GPIO_BASE(n) ((GPIO_TypeDef  *)(GPIOA_BASE + (GPIO_PORT(n) * 0x400)))

//-----------------------------------------------------------------------------

typedef struct gpio_info {
	uint32_t num;		// gpio number
	uint32_t mode;		// input, output, etc.
	uint32_t pull;		// pull up/down, etc.
	uint32_t speed;		// slew rate
	uint32_t alt;		// alternate pin functions
	int init;		// initial pin value
} GPIO_INFO;

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

void gpio_init(const GPIO_INFO * info, size_t n);

//-----------------------------------------------------------------------------

#endif				// GPIO_H

//-----------------------------------------------------------------------------
