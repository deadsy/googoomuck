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

#define IO_PUSH_BUTTON    GPIO_NUM(PORTA, 10)	// GPIO: pushbutton (0=open,1=pressed)
#define IO_UART_TX        GPIO_NUM(PORTG, 14)	// AF8: serial port tx
#define IO_UART_RX        GPIO_NUM(PORTG, 9)	// AF8: serial port rx
#define IO_LED_GREEN      GPIO_NUM(PORTG, 6)	// GPIO: green led
#define IO_LED_RED        GPIO_NUM(PORTC, 6)	// GPIO: red led
#define IO_AUDIO_I2C_SCL  GPIO_NUM(PORTH, 7)	// GPIO: I2C clock (bitbanged)
#define IO_AUDIO_I2C_SDA  GPIO_NUM(PORTH, 8)	// GPIO: I2C data (bitbanged)

//#define IO_AUDIO_I2S_WS GPIO_NUM(PORTE, 4)    // AF6: I2S3 channel clock
//#define IO_AUDIO_I2S_MCK GPIO_NUM(PORTC, 7)   // AF6: I2S3 master clock
//#define IO_AUDIO_I2S_SCK GPIO_NUM(PORTC, 10)  // AF6: I2S3 serial clock
//#define IO_AUDIO_I2S_SD GPIO_NUM(PORTC, 12)   // AF6: I2S3 serial data

//-----------------------------------------------------------------------------

#endif				// IO_H

//-----------------------------------------------------------------------------
