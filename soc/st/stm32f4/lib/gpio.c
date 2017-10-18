//-----------------------------------------------------------------------------
/*

GPIO Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

// set the pin mode
static void set_mode(GPIO_TypeDef * gpio, int pin, int mode) {
	int shift = pin << 1;
	uint32_t val = gpio->MODER;
	val &= ~(3 << shift);
	val |= mode << shift;
	gpio->MODER = val;
}

// set the output type
static void set_otype(GPIO_TypeDef * gpio, int pin, int type) {
	uint32_t val = gpio->OTYPER;
	val &= ~(1 << pin);
	val |= type << pin;
	gpio->OTYPER = val;
}

// set the pull-up/pull-down mode
static void set_pupd(GPIO_TypeDef * gpio, int pin, int pupd) {
	int shift = pin << 1;
	uint32_t val = gpio->PUPDR;
	val &= ~(3 << shift);
	val |= pupd << shift;
	gpio->PUPDR = val;
}

// set the output speed
static void set_ospeed(GPIO_TypeDef * gpio, int pin, int speed) {
	int shift = pin << 1;
	uint32_t val = gpio->OSPEEDR;
	val &= ~(3 << shift);
	val |= speed << shift;
	gpio->OSPEEDR = val;
}

// set the alternate function number
static void set_altfunc(GPIO_TypeDef * gpio, int pin, int af) {
	volatile uint32_t *reg;
	uint32_t val;
	int shift;
	if (pin < 8) {
		reg = &gpio->AFR[0];
	} else {
		reg = &gpio->AFR[1];
		pin -= 8;
	}
	shift = pin << 2;
	val = *reg;
	val &= ~(15 << shift);
	val |= af << shift;
	*reg = val;
}

//-----------------------------------------------------------------------------

static void gpio_enable(int port) {
	// This is for stm32f407
	// It is different for other SoCs in the STM family
	RCC->AHB1ENR |= (1 << port);	// clock enable
	RCC->AHB1LPENR &= ~(1 << port);	// clock enable during sleep mode
}

//-----------------------------------------------------------------------------

int gpio_init(const struct gpio_info *info, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		const struct gpio_info *x = &info[i];
		GPIO_TypeDef *gpio = GPIO_BASE(x->num);
		int pin = GPIO_PIN(x->num);
		gpio_enable(GPIO_PORT(x->num));
		set_mode(gpio, pin, x->mode);
		set_otype(gpio, pin, x->type);
		set_ospeed(gpio, pin, x->speed);
		set_pupd(gpio, pin, x->pupd);
		set_altfunc(gpio, pin, x->af);
		if (x->init == 0) {
			gpio_clr(x->num);
		}
		if (x->init == 1) {
			gpio_set(x->num);
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
