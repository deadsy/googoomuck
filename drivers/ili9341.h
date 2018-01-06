//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

*/
//-----------------------------------------------------------------------------

#ifndef ILI9341_H
#define ILI9341_H

//-----------------------------------------------------------------------------

#include <stddef.h>
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
// LCD operations queue

#define NUM_OPS 32		// must be a power of 2

enum {
	LCD_OP_NOP,		// 0, no operation
	LCD_OP_CSASSERT,	// chip selected
	LCD_OP_CSDEASSERT,	// chip deselected
	LCD_OP_DATAMODE,	// switch to datamode
	LCD_OP_CMD,		// command byte
	LCD_OP_TXBUF8,		// n x uint8_t (buffer)
	LCD_OP_TX8,		// n x uint8_t (same value)
	LCD_OP_TX16,		// n x uint16_t (same value)
};

struct lcd_op_cmd {
	uint8_t cmd;
};

struct lcd_op_txbuf8 {
	size_t n;
	const uint8_t *buf;
};

struct lcd_op_tx8 {
	size_t n;
	uint8_t data;
};

struct lcd_op_tx16 {
	size_t n;
	uint16_t data;
};

struct lcd_op {
	uint8_t type;		// type of lcd operation
	union op_data {
		struct lcd_op_cmd cmd;
		struct lcd_op_txbuf8 txbuf8;
		struct lcd_op_tx8 tx8;
		struct lcd_op_tx16 tx16;
	} u;
};

struct lcd_op_queue {
	struct lcd_op queue[NUM_OPS];
	size_t rd;
	size_t wr;
};

//-----------------------------------------------------------------------------

struct lcd_cfg {
	struct spi_drv *spi;	// spi bus
	int rst;		// gpio for reset pin
	int dc;			// gpio for d/c line
	int cs;			// gpio for chip select
	int led;		// gpio for led backlight control
	int rotation;		// screen rotation
	uint16_t bg;		// background color
};

struct lcd_drv {
	struct lcd_cfg cfg;
	struct lcd_op_queue opq;
	int width, height;	// screen width/height in pixels
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
