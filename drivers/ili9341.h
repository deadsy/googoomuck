//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

*/
//-----------------------------------------------------------------------------

#ifndef ILI9341_H
#define ILI9341_H

//-----------------------------------------------------------------------------

struct ili9341_cfg {
	struct spi_drv *spi;	// spi bus
	int rst;		// gpio for reset pin
	int dc;			// gpio for d/c line
	int cs;			// gpio for chip select
	int led;		// gpio for led backlight control
};

struct ili9341_drv {
	struct ili9341_cfg cfg;
	int width;		// width in pixels
	int height;		// height in pixels
};

//-----------------------------------------------------------------------------

int ili9341_init(struct ili9341_drv *lcd, struct ili9341_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// ILI9341_H

//-----------------------------------------------------------------------------
