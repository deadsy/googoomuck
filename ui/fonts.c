//-----------------------------------------------------------------------------
/*

Font Functions

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "lcd.h"

//-----------------------------------------------------------------------------
// available fonts

extern const struct font nokia_large;

static const struct font *fonts[] = {
	&nokia_large,
};

//-----------------------------------------------------------------------------

void lcd_string(struct lcd_drv *drv, uint16_t x, uint16_t y, int font, uint16_t fg, uint16_t bg, char *str) {
	const struct font *f = fonts[font];
	for (size_t i = 0; i < strlen(str); i++) {
		const struct glyph *g = &f->glyphs[(uint8_t) str[i]];
		lcd_draw_bitmap(drv, x + g->xofs, y - g->yofs - g->height, g->width, g->height, fg, bg, g->data);
		x += (g->dwidth + 1);
	}
}

//-----------------------------------------------------------------------------

// Reset the cursor position, set the current font.
void lcd_set_font(struct lcd_drv *drv, int font) {
	drv->font = fonts[font];
	drv->x = 0;
	drv->y = drv->font->ascent;
}

// Print a string at the current cursor position.
// The lcd is used like a terminal.
void lcd_print(struct lcd_drv *drv, char *str) {
	for (size_t i = 0; i < strlen(str); i++) {
		if (str[i] == '\n') {
			drv->x = 0;
			drv->y += (drv->font->ascent - drv->font->descent);
		} else {
			const struct glyph *g = &drv->font->glyphs[(uint8_t) str[i]];
			uint16_t bx = drv->x + g->xofs;
			uint16_t by = drv->y - g->yofs - g->height;
			lcd_draw_bitmap(drv, bx, by, g->width, g->height, drv->cfg.fg, drv->cfg.bg, g->data);
			drv->x += (g->dwidth + 1);
		}
	}
}

//-----------------------------------------------------------------------------
