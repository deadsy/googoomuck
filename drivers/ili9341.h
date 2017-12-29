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

#define LCD_COLOR_BLACK       0x0000	/*   0,   0,   0 */
#define LCD_COLOR_NAVY        0x000F	/*   0,   0, 128 */
#define LCD_COLOR_DARKGREEN   0x03E0	/*   0, 128,   0 */
#define LCD_COLOR_DARKCYAN    0x03EF	/*   0, 128, 128 */
#define LCD_COLOR_MAROON      0x7800	/* 128,   0,   0 */
#define LCD_COLOR_PURPLE      0x780F	/* 128,   0, 128 */
#define LCD_COLOR_OLIVE       0x7BE0	/* 128, 128,   0 */
#define LCD_COLOR_LIGHTGREY   0xC618	/* 192, 192, 192 */
#define LCD_COLOR_DARKGREY    0x7BEF	/* 128, 128, 128 */
#define LCD_COLOR_BLUE        0x001F	/*   0,   0, 255 */
#define LCD_COLOR_GREEN       0x07E0	/*   0, 255,   0 */
#define LCD_COLOR_CYAN        0x07FF	/*   0, 255, 255 */
#define LCD_COLOR_RED         0xF800	/* 255,   0,   0 */
#define LCD_COLOR_MAGENTA     0xF81F	/* 255,   0, 255 */
#define LCD_COLOR_YELLOW      0xFFE0	/* 255, 255,   0 */
#define LCD_COLOR_WHITE       0xFFFF	/* 255, 255, 255 */
#define LCD_COLOR_ORANGE      0xFD20	/* 255, 165,   0 */
#define LCD_COLOR_GREENYELLOW 0xAFE5	/* 173, 255,  47 */
#define LCD_COLOR_PINK        0xF81F

//-----------------------------------------------------------------------------

struct lcd_cfg {
	struct spi_drv *spi;	// spi bus
	int rst;		// gpio for reset pin
	int dc;			// gpio for d/c line
	int cs;			// gpio for chip select
	int led;		// gpio for led backlight control
	uint16_t fg;		// foreground color
	uint16_t bg;		// background color
	int rotation;		// screen rotation
};

struct lcd_drv {
	struct lcd_cfg cfg;
	int width, height;	// screen width/height in pixels
	int x, y;		// current cursor position
	const struct font *font;	// current font
};

//-----------------------------------------------------------------------------

int lcd_init(struct lcd_drv *lcd, struct lcd_cfg *cfg);
void lcd_fill_rect(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void lcd_set_pixel(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t color);
void lcd_draw_bitmap(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t bg, const uint32_t * buf);
void lcd_set_scroll_region(struct lcd_drv *drv, uint16_t tfa, uint16_t vsa);
void lcd_scroll(struct lcd_drv *drv, uint16_t vsp);

//-----------------------------------------------------------------------------

#endif				// ILI9341_H

//-----------------------------------------------------------------------------
