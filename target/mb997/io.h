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

#define IO_PUSH_BUTTON    GPIO_NUM(PORTA, 0)	// GPIO: pushbutton (0=open,1=pressed)
#define IO_UART_TX        GPIO_NUM(PORTA, 2)	// AF7: serial port tx
#define IO_UART_RX        GPIO_NUM(PORTA, 3)	// AF7: serial port rx
#define IO_AUDIO_I2S_WS   GPIO_NUM(PORTA, 4)	// AF6: I2S3 channel clock

#define IO_AUDIO_I2C_SCL  GPIO_NUM(PORTB, 6)	// GPIO: I2C clock (bitbanged)
#define IO_AUDIO_I2C_SDA  GPIO_NUM(PORTB, 9)	// GPIO: I2C data (bitbanged)
#define IO_LCD_CS         GPIO_NUM(PORTB, 12)	// GPIO: lcd driver chip select
#define IO_LCD_SCK        GPIO_NUM(PORTB, 13)	// AF5: spi2 clock
#define IO_LCD_SDO        GPIO_NUM(PORTB, 14)	// AF5: spi2 data MISO
#define IO_LCD_SDI        GPIO_NUM(PORTB, 15)	// AF5: spi2 data MOSI

#define IO_AUDIO_I2S_MCK  GPIO_NUM(PORTC, 7)	// AF6: I2S3 master clock
#define IO_AUDIO_I2S_SCK  GPIO_NUM(PORTC, 10)	// AF6: I2S3 serial clock
#define IO_AUDIO_I2S_SD   GPIO_NUM(PORTC, 12)	// AF6: I2S3 serial data

#define IO_AUDIO_RESET    GPIO_NUM(PORTD, 4)	// GPIO: reset line to stereo DAC
#define IO_LCD_DATA_CMD   GPIO_NUM(PORTD, 6)	// GPIO: lcd data/command select
#define IO_LCD_RESET      GPIO_NUM(PORTD, 7)	// GPIO: lcd driver chip reset
#define IO_LCD_LED        GPIO_NUM(PORTD, 8)	// GPIO: lcd backlight
#define IO_LED_GREEN      GPIO_NUM(PORTD, 12)	// GPIO: green led
#define IO_LED_AMBER      GPIO_NUM(PORTD, 13)	// GPIO: amber led
#define IO_LED_RED        GPIO_NUM(PORTD, 14)	// GPIO: red led
#define IO_LED_BLUE       GPIO_NUM(PORTD, 15)	// GPIO: blue led

//-----------------------------------------------------------------------------

#endif				// IO_H

//-----------------------------------------------------------------------------
