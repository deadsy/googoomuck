//-----------------------------------------------------------------------------
/*

Font Functions

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "lcd.h"

#define DEBUG
#include "logging.h"

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

// use a region of the screen for stdio output
void lcd_terminal_init(struct lcd_drv *drv, uint16_t y, int lines, int font) {
	drv->line = 0;
	drv->maxline = lines - 1;
	drv->scrolling = 0;
	drv->font = fonts[font];
	drv->dy = drv->font->ascent - drv->font->descent;
	drv->y0 = y + drv->font->ascent;
	drv->x = 0;
	drv->y = drv->y0;
	lcd_set_scroll_region(drv, y, lines * drv->dy);
}

// Print a string at the current cursor position.
// The lcd is used like a terminal.
void lcd_print(struct lcd_drv *drv, char *str) {
	for (size_t i = 0; i < strlen(str); i++) {
		if (str[i] == '\n') {
			// increment the line
			if (drv->line == drv->maxline) {
				drv->scrolling = 1;
				drv->line = 0;
			} else {
				drv->line += 1;
			}
			// set the cursor
			drv->x = 0;
			drv->y = drv->y0 + (drv->line * drv->dy);
			// clear the line
			lcd_fill_rect(drv, drv->x, drv->y - drv->y0, drv->width, drv->dy, drv->cfg.bg);
			// scroll the screen
			if (drv->scrolling) {
				lcd_scroll(drv, drv->y - drv->y0 + drv->dy);
			}
		} else {
			const struct glyph *g = &drv->font->glyphs[(uint8_t) str[i]];
			uint16_t bx = drv->x + g->xofs;
			uint16_t by = drv->y - g->yofs - g->height;
			lcd_draw_bitmap(drv, bx, by, g->width, g->height, drv->cfg.fg, drv->cfg.bg, g->data);
			drv->x += g->dwidth;
		}
	}
}

//-----------------------------------------------------------------------------
