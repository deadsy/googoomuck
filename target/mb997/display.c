//-----------------------------------------------------------------------------
/*

Display Control

*/
//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// IO configuration

#define LCD_SDO       GPIO_NUM(PORTA, 6)	// spi data MISO
#define LCD_SCK       GPIO_NUM(PORTA, 5)	// spi clock
#define LCD_SDI       GPIO_NUM(PORTA, 7)	// spi data MOSI
#define LCD_CS        GPIO_NUM(PORTA, 15)	// chip select
#define LCD_DATA_CMD  GPIO_NUM(PORTD, 6)	// lcd data/command select
#define LCD_RESET     GPIO_NUM(PORTD, 7)	// reset pin

static const struct gpio_info gpios[] = {
	{LCD_SDO, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF5, 0},
	{LCD_SCK, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF5, 0},
	{LCD_SDI, GPIO_MODER_AF, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF5, 0},
	{LCD_DATA_CMD, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LCD_RESET, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_LO, GPIO_PUPD_NONE, GPIO_AF0, 0},
	{LCD_CS, GPIO_MODER_OUT, GPIO_OTYPER_PP, GPIO_OSPEEDR_FAST, GPIO_PUPD_NONE, GPIO_AF0, 0},
};

//-----------------------------------------------------------------------------

int display_init(void) {
	int rc = 0;

	// setup the io pins
	rc = gpio_init(gpios, sizeof(gpios) / sizeof(struct gpio_info));
	if (rc != 0) {
		DBG("gpio_init failed %d\r\n", rc);
		goto exit;
	}

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
