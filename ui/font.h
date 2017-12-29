//-----------------------------------------------------------------------------
/*

Font Encoding

*/
//-----------------------------------------------------------------------------

#ifndef FONT_H
#define FONT_H

//-----------------------------------------------------------------------------

#include <inttypes.h>

#include "ili9341.h"

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void lcd_string(struct lcd_drv *drv, uint16_t x, uint16_t y, int font, uint16_t fg, uint16_t bg, char *str);
void lcd_set_font(struct lcd_drv *drv, int font);
void lcd_print(struct lcd_drv *drv, char *str);

//-----------------------------------------------------------------------------

#endif				// FONT_H

//-----------------------------------------------------------------------------
