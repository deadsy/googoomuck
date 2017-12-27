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

#if defined(SPI_DRIVER_HW)
static struct spi_cfg lcd_spi_cfg = {
	.base = SPI2_BASE,
	.mode = SPI_MODE_MASTER,
	.cpol = SPI_CPOL_LO,	// clock is normally low
	.cpha = SPI_CPHA_CLK1,	// data valid on 1st clock edge
	.lsb = SPI_MSB_FIRST,	// ms bit first
	.div = SPI_BAUD_DIV2,	// spi_clock = 168 MHz / 4 * divider
};
#elif defined(SPI_DRIVER_BITBANG)
static struct spi_cfg lcd_spi_cfg = {
	.clk = IO_LCD_SCK,
	.mosi = IO_LCD_SDI,
	.miso = IO_LCD_SDO,
	.cpol = 0,		// clock is normally low
	.cpha = 1,		// latch MISO on falling clock edge
	.lsb = 0,		// ms bit first
	.delay = 0,
};
#else
#error "what kind of SPI driver are we building?"
#endif

//-----------------------------------------------------------------------------

static struct ili9341_cfg lcd_cfg = {
	.rst = IO_LCD_RESET,	// gpio for reset pin
	.dc = IO_LCD_DATA_CMD,	// gpio for d/c line
	.cs = IO_LCD_CS,	// gpio for chip select
	.led = IO_LCD_LED,	// gpio for led backlight control
};

//-----------------------------------------------------------------------------

#define TOFS 10
#define TSIZE 100

static void lcd_test(struct ili9341_drv *drv) {
	//uint16_t x, y;

#if 0

	// top left
	x = TOFS;
	y = TOFS;
	lcd_fill_rect(drv, x, y, TSIZE, TSIZE, ILI9341_RED);

	// top right
	x = drv->width - TOFS - TSIZE;
	y = TOFS;
	lcd_fill_rect(drv, x, y, TSIZE, TSIZE, ILI9341_GREEN);

	// bottom right
	x = drv->width - TOFS - TSIZE;
	y = drv->height - TOFS - TSIZE;
	lcd_fill_rect(drv, x, y, TSIZE, TSIZE, ILI9341_WHITE);

	// bottom left
	x = TOFS;
	y = drv->height - TOFS - TSIZE;
	lcd_fill_rect(drv, x, y, TSIZE, TSIZE, ILI9341_BLUE);

#endif

	for (int x = 0; x < 320; x++) {
		lcd_fill_rect(drv, x, 0, 1, x, ILI9341_ORANGE);
	}

	//lcd_draw_rect(drv, TOFS, TOFS, drv->width - (2 * TOFS), drv->height - (2 * TOFS), ILI9341_ORANGE);
}

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

	lcd_test(&display->lcd);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
