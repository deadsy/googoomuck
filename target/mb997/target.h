//-----------------------------------------------------------------------------
/*

Target Defines

*/
//-----------------------------------------------------------------------------

#ifndef TARGET_H
#define TARGET_H

//-----------------------------------------------------------------------------

#include "gpio.h"

//-----------------------------------------------------------------------------

// standard board GPIO
#define LED_GREEN       GPIO_NUM(PORTD, 12)
#define LED_AMBER       GPIO_NUM(PORTD, 13)
#define LED_RED         GPIO_NUM(PORTD, 14)
#define LED_BLUE        GPIO_NUM(PORTD, 15)
#define PUSH_BUTTON     GPIO_NUM(PORTA, 0)	// 0 = open, 1 = pressed

// i2c bus
#define GPIO_I2C_SCL    GPIO_NUM(PORTB, 6)
#define GPIO_I2C_SDA    GPIO_NUM(PORTB, 9)

//-----------------------------------------------------------------------------

#endif				// TARGET_H

//-----------------------------------------------------------------------------
