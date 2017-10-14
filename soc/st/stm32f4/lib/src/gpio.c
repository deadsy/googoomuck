//-----------------------------------------------------------------------------
/*

GPIO Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#include "gpio.h"

//-----------------------------------------------------------------------------

void gpio_init(const GPIO_INFO * info, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		const GPIO_INFO *gpio = &info[i];
		GPIO_InitTypeDef GPIO_InitStruct;
		// enable the peripheral clock: __GPIOx_CLK_ENABLE()
		RCC->AHB1ENR |= (1 << GPIO_PORT(gpio->num));
		// setup the gpio port/pin
		GPIO_InitStruct.Pin = GPIO_BIT(gpio->num);
		GPIO_InitStruct.Mode = gpio->mode;
		GPIO_InitStruct.Pull = gpio->pull;
		GPIO_InitStruct.Speed = gpio->speed;
		GPIO_InitStruct.Alternate = gpio->alt;
		HAL_GPIO_Init(GPIO_BASE(gpio->num), &GPIO_InitStruct);
		// set any initial value
		if (gpio->init >= 0) {
			HAL_GPIO_WritePin(GPIO_BASE(gpio->num), GPIO_BIT(gpio->num), gpio->init);
		}
	}
}

//-----------------------------------------------------------------------------
