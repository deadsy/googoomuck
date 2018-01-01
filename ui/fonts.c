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

// initialise a region of the lcd to be used for stdio output
int term_init(struct term_drv *drv, struct term_cfg *cfg) {
	memset(drv, 0, sizeof(struct term_drv));
	drv->cfg = *cfg;
	drv->font = fonts[drv->cfg.font];
	drv->dy = drv->font->ascent - drv->font->descent;
	drv->y0 = drv->cfg.yofs + drv->font->ascent;
	drv->y = drv->y0;
	// setup the lcd
	uint16_t h = drv->cfg.lines * drv->dy;
	lcd_set_scroll_region(drv->cfg.lcd, drv->cfg.yofs, h);
	lcd_fill_rect(drv->cfg.lcd, 0, drv->cfg.yofs, drv->cfg.lcd->width, h, drv->cfg.bg);
	return 0;
}

// Print a string at the current cursor position.
void term_print(struct term_drv *drv, char *str) {
	for (size_t i = 0; i < strlen(str); i++) {
		if (str[i] == '\n') {
			// Process the CR when we see the next character.
			// This prevents the empty-last-line issue.
			drv->cr_flag = 1;
		} else {
			// handle the previous CR
			if (drv->cr_flag) {
				// increment the line
				drv->line += 1;
				if (drv->line == drv->cfg.lines) {
					drv->scrolling = 1;
					drv->line = 0;
				}
				// set the cursor
				drv->x = 0;
				drv->y = drv->y0 + (drv->line * drv->dy);
				// clear the line
				lcd_fill_rect(drv->cfg.lcd, drv->x, drv->y - drv->y0, drv->cfg.lcd->width, drv->dy, drv->cfg.bg);
				// scroll the screen
				if (drv->scrolling) {
					lcd_scroll(drv->cfg.lcd, drv->y - drv->y0 + drv->dy);
				}
				drv->cr_flag = 0;
			}
			// print the character
			const struct glyph *g = &drv->font->glyphs[(uint8_t) str[i]];
			uint16_t bx = drv->x + g->xofs;
			uint16_t by = drv->y - g->yofs - g->height;
			lcd_draw_bitmap(drv->cfg.lcd, bx, by, g->width, g->height, drv->cfg.fg, drv->cfg.bg, g->data);
			drv->x += g->dwidth;
		}
	}
}

//-----------------------------------------------------------------------------
