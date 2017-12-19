//-----------------------------------------------------------------------------
/*

Display Control

*/
//-----------------------------------------------------------------------------

#include "display.h"
#include "io.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct display_drv ggm_display;

//-----------------------------------------------------------------------------
// SPI Setup

static struct spi_cfg lcd_spi_cfg = {
	.base = SPI1_BASE,
	.mode = SPI_MODE_MST,
	.cpol = SPI_CPOL_HI,
	.cpha = SPI_CPHA_CLK2,
	.ff = SPI_FF_8B_MSB,
	.div = SPI_BAUD_DIV64,
};

//-----------------------------------------------------------------------------

static struct ili9341_cfg lcd_cfg = {
	.rst = IO_LCD_RESET,	// gpio for reset pin
	.dc = IO_LCD_DATA_CMD,	// gpio for d/c line
	.cs = IO_LCD_CS,	// gpio for chip select
	.led = IO_LCD_LED,	// gpio for led backlight control
};

//-----------------------------------------------------------------------------

int display_init(struct display_drv *display) {
	int rc = 0;
	// setup the lcd spi bus
	rc = spi_init(&display->spi, &lcd_spi_cfg);
	if (rc != 0) {
		DBG("spi_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the lcd
	lcd_cfg.spi = &display->spi;
	rc = ili9341_init(&display->lcd, &lcd_cfg);
	if (rc != 0) {
		DBG("ili9341_init failed %d\r\n", rc);
		goto exit;
	}

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
