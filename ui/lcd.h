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

const struct font *font_get(int idx);

//-----------------------------------------------------------------------------
// STDIO to LCD terminal driver

struct term_cfg {
	struct lcd_drv *lcd;	// lcd driver
	int font;		// font being used
	int lines;		// number of lines
	int yofs;		// y position on screen
	uint16_t bg;		// background color
	uint16_t fg;		// foreground color
};

struct term_drv {
	struct term_cfg cfg;
	const struct font *font;	// current font
	int x, y;		// current cursor position
	int y0;			// top position for the terminal window
	int dy;			// line height for the current font
	int line;		// current line number
	int scrolling;		// are we scrolling yet?
	int cr_flag;		// cr to process?
};

int term_init(struct term_drv *drv, struct term_cfg *cfg);
void term_print(struct term_drv *drv, char *str);

//-----------------------------------------------------------------------------
// graphics

void lcd_string(struct lcd_drv *drv, uint16_t x, uint16_t y, int font, uint16_t fg, uint16_t bg, char *str);
void lcd_fill_screen(struct lcd_drv *drv, uint16_t color);
void lcd_draw_vline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void lcd_draw_hline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void lcd_draw_rect(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

//-----------------------------------------------------------------------------

#endif				// LCD_H

//-----------------------------------------------------------------------------
