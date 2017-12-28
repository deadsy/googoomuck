//-----------------------------------------------------------------------------
/*

Font Functions

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "font.h"

//-----------------------------------------------------------------------------
// available fonts

extern const struct font nokia_large;

static const struct font *fonts[] = {
	&nokia_large,
};

//-----------------------------------------------------------------------------

void font_draw_string(struct lcd_drv *drv, uint16_t x, uint16_t y, uint8_t font, uint16_t fg, uint16_t bg, char *str) {
	const struct font *f = fonts[font];
	for (size_t i = 0; i < strlen(str); i++) {
		const struct glyph *g = &f->glyphs[(uint8_t) str[i]];
		lcd_draw_bitmap(drv, x + g->xofs, y - g->yofs - g->height, g->width, g->height, fg, bg, g->data);
		x += g->dwidth;
	}
}

//-----------------------------------------------------------------------------
