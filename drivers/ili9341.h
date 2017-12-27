//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

*/
//-----------------------------------------------------------------------------

#ifndef ILI9341_H
#define ILI9341_H

//-----------------------------------------------------------------------------

#include <inttypes.h>

//-----------------------------------------------------------------------------
// colors

#define ILI9341_BLACK       0x0000	/*   0,   0,   0 */
#define ILI9341_NAVY        0x000F	/*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0	/*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF	/*   0, 128, 128 */
#define ILI9341_MAROON      0x7800	/* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F	/* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0	/* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618	/* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF	/* 128, 128, 128 */
#define ILI9341_BLUE        0x001F	/*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0	/*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF	/*   0, 255, 255 */
#define ILI9341_RED         0xF800	/* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F	/* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0	/* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF	/* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20	/* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5	/* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

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

void lcd_fill_rect(struct ili9341_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void lcd_set_pixel(struct ili9341_drv *drv, uint16_t x, uint16_t y, uint16_t color);
void lcd_fill_screen(struct ili9341_drv *drv, uint16_t color);
void lcd_draw_vline(struct ili9341_drv *drv, uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void lcd_draw_hline(struct ili9341_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void lcd_draw_rect(struct ili9341_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

//-----------------------------------------------------------------------------

#endif				// ILI9341_H

//-----------------------------------------------------------------------------
