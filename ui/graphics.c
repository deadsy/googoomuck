//-----------------------------------------------------------------------------
/*

Graphics Functions

*/
//-----------------------------------------------------------------------------

#include "ui.h"

//-----------------------------------------------------------------------------

// fill the screen with a color
void lcd_fill_screen(struct lcd_drv *drv, uint16_t color) {
	lcd_fill_rect(drv, 0, 0, drv->width, drv->height, color);
}

// draw a vertical line
void lcd_draw_vline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
	lcd_fill_rect(drv, x, y, 1, h, color);
}

// draw a horizontal line
void lcd_draw_hline(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
	lcd_fill_rect(drv, x, y, w, 1, color);
}

// draw a rectangle
void lcd_draw_rect(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	if (w == 0 || h == 0) {
		return;
	}
	if (w == 1) {
		lcd_draw_vline(drv, x, y, h, color);
		return;
	}
	if (h == 1) {
		lcd_draw_hline(drv, x, y, w, color);
		return;
	}
	lcd_draw_hline(drv, x, y, w, color);
	lcd_draw_hline(drv, x, y + h - 1, w, color);
	lcd_draw_vline(drv, x, y, h, color);
	lcd_draw_vline(drv, x + w - 1, y, h, color);
}

//-----------------------------------------------------------------------------
