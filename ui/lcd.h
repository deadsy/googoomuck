//-----------------------------------------------------------------------------
/*

LCD Graphics Functions

*/
//-----------------------------------------------------------------------------

#ifndef LCD_H
#define LCD_H

//-----------------------------------------------------------------------------

#include <inttypes.h>

#include "ili9341.h"

//-----------------------------------------------------------------------------
// Fonts

struct glyph {
	int8_t width;
	int8_t height;
	int8_t xofs;
	int8_t yofs;
	int8_t dwidth;
	const uint32_t *data;
};

struct font {
	int8_t ascent;
	int8_t descent;
	struct glyph glyphs[256];
};

void lcd_string(struct lcd_drv *drv, uint16_t x, uint16_t y, int font, uint16_t fg, uint16_t bg, char *str);
void lcd_set_font(struct lcd_drv *drv, int font);
void lcd_print(struct lcd_drv *drv, char *str);

//-----------------------------------------------------------------------------
// graphics

void lcd_fill_screen(struct lcd_drv *drv, uint16_t color);
void lcd_draw_vline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void lcd_draw_hline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void lcd_draw_rect(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

//-----------------------------------------------------------------------------

#endif				// LCD_H

//-----------------------------------------------------------------------------
