//-----------------------------------------------------------------------------
/*

IO Pin Assignments

*/
//-----------------------------------------------------------------------------

#ifndef IO_H
#define IO_H

//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------

#define IO_SWITCH_1       GPIO_NUM(PORTB, 5)	// GPIO: pushbutton (0=open,1=pressed)
#define IO_SWITCH_2       GPIO_NUM(PORTA, 10)	// GPIO: pushbutton (0=open,1=pressed)
#define IO_UART_TX        GPIO_NUM(PORTG, 14)	// AF8: serial port tx
#define IO_UART_RX        GPIO_NUM(PORTG, 9)	// AF8: serial port rx
#define IO_LED_GREEN      GPIO_NUM(PORTG, 6)	// GPIO: green led
#define IO_LED_RED        GPIO_NUM(PORTC, 6)	// GPIO: red led
#define IO_AUDIO_I2C_SCL  GPIO_NUM(PORTH, 7)	// GPIO: I2C clock (bitbanged)
#define IO_AUDIO_I2C_SDA  GPIO_NUM(PORTH, 8)	// GPIO: I2C data (bitbanged)
#define IO_AUDIO_MCLK     GPIO_NUM(PORTA, 8)	// AF0: MCO1
#define IO_AUDIO_ADC      GPIO_NUM(PORTE, 3)	// AF6: SAI1_SD_B
#define IO_AUDIO_LRCLK    GPIO_NUM(PORTE, 4)	// AF6: SAI1_FS_A
#define IO_AUDIO_BCLK     GPIO_NUM(PORTE, 5)	// AF6: SAI1_SCK_A
#define IO_AUDIO_DAC      GPIO_NUM(PORTE, 6)	// AF6: SAI1_SD_A

//-----------------------------------------------------------------------------

#endif				// IO_H

//-----------------------------------------------------------------------------
