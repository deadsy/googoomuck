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

static struct lcd_cfg lcd_cfg = {
	.rst = IO_LCD_RESET,	// gpio for reset pin
	.dc = IO_LCD_DATA_CMD,	// gpio for d/c line
	.cs = IO_LCD_CS,	// gpio for chip select
	.led = IO_LCD_LED,	// gpio for led backlight control
	.bg = LCD_COLOR_NAVY,
	.rotation = 0,
};

//-----------------------------------------------------------------------------

static struct term_cfg term_cfg = {
	.font = 0,
	.lines = 22,
	.yofs = 0,
	.bg = LCD_COLOR_NAVY,
	.fg = LCD_COLOR_WHITE,
};

//-----------------------------------------------------------------------------

static void term_test(struct term_drv *drv) {

	term_print(drv, "GooGooMuck!");

#if 0
	while (1) {
		term_print(drv, "line 0 asdfasdfasdfasfd\n");
		mdelay(100);
		term_print(drv, "line 1 asdfasfd\n");
		mdelay(100);
		term_print(drv, "line 2 12312341234\n");
		mdelay(100);
		term_print(drv, "line 3 xcvbxcvb\n");
		mdelay(100);
		term_print(drv, "line 4 ghjkghkj\n");
		mdelay(100);
		term_print(drv, "line 5 &*&(&(\n");
		mdelay(100);
		term_print(drv, "line 6 [][][]\n");
		mdelay(100);
		term_print(drv, "line 7 @@##$$\n");
		mdelay(100);
		term_print(drv, "line 8 zxcvzxv\n");
		mdelay(100);
	}
#endif

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
	rc = lcd_init(&display->lcd, &lcd_cfg);
	if (rc != 0) {
		DBG("lcd_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the stdio terminal
	term_cfg.lcd = &display->lcd;
	rc = term_init(&display->term, &term_cfg);
	if (rc != 0) {
		DBG("term_init failed %d\r\n", rc);
		goto exit;
	}

	term_test(&display->term);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
