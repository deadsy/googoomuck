//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

This is for a 4 wire SPI interface.

Pins are:

SDO (SPI MISO)      IO_LCD_SDO
SCK (SPI Clock)     IO_LCD_SCK
SDI (SPI MOSI)      IO_LCD_SDI
D/C (data/command)  IO_LCD_DATA_CMD, data=1, command=0
RESET (reset line)  IO_LCD_RESET, reset=0, normal=1
CS (chip select)    IO_LCD_CS, assert=0, deassert=1
LED                 IO_LCD_LED, on=1, off=0
GND                 Ground
VCC                 3.3v

Operation:

This driver queues a set of operations for the LCD driver chip.
Processing of the operations is interrupt driven. Either an interrupt
on the SPI tx buffer empty or an interrupt on the SPI tx operation
completion. The high level code can add operations to the queue without
waiting for hardware. Upon the HW interrupt the queue will be processed.
Typically a DMA will be setup to write data to the device.

Notes:

Don't change the CS line or D/C line until the SPI operation is complete.
This isn't a problem for bit banged SPI, but in HW based SPI the SPI and
GPIO operations are asynchronous, so you have to wait for SPI completion.

The ili9341 supports hardware scrolling. This remaps the way the driver
moves the graphics ram onto the display. This only works on the long axis of
the LCD (320 pixels). If you want to use it for scrolling a screen of
text, you'll have to be in portrait mode (rotation = 0/2).

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ili9341.h"
#include "stm32f4_soc.h"

//-----------------------------------------------------------------------------
// Command Set

// regular
#define CMD_NOP                     0x00
#define CMD_SW_RST                  0x01
#define CMD_RD_DISP_ID              0x04
#define CMD_RD_DISP_STATUS          0x09
#define CMD_RD_DISP_POWER_MODE      0x0a
#define CMD_RD_DISP_MADCTL          0x0b
#define CMD_RD_DISP_PIXEL_FMT       0x0c
#define CMD_RD_DISP_IMAGE_FMT       0x0d
#define CMD_RD_DISP_SIGNAL_MODE     0x0e
#define CMD_RD_DISP_SELF_DIAG       0x0f
#define CMD_ENTER_SLEEP_MODE        0x10
#define CMD_SLEEP_OUT               0x11
#define CMD_PARTIAL_MODE_ON         0x12
#define CMD_NORMAL_DISP_MODE_ON     0x13
#define CMD_DISP_INVERSION_OFF      0x20
#define CMD_DISP_INVERSION_ON       0x21
#define CMD_GAMMA_SET               0x26
#define CMD_DISP_OFF                0x28
#define CMD_DISP_ON                 0x29
#define CMD_COLUMN_ADDR_SET         0x2a
#define CMD_PAGE_ADDR_SET           0x2b
#define CMD_MEM_WR                  0x2c
#define CMD_COLOR_SET               0x2d
#define CMD_MEM_RD                  0x2e
#define CMD_PARTIAL_AREA            0x30
#define CMD_VSCROLL_DEFN            0x33
#define CMD_TEARING_EFFECT_LINE_OFF 0x34
#define CMD_TEARING_EFFECT_LINE_ON  0x35
#define CMD_MEM_ACCESS_CTRL         0x36
#define CMD_VSCROLL_START_ADDR      0x37
#define CMD_IDLE_MODE_OFF           0x38
#define CMD_IDLE_MODE_ON            0x39
#define CMD_PIXEL_FMT_SET           0x3a
#define CMD_WR_MEM_CONT             0x3c
#define CMD_RD_MEM_CONT             0x3e
#define CMD_SET_TEAR_SCANLINE       0x44
#define CMD_GET_SCANLINE            0x45
#define CMD_WR_DISP_BRIGHTNESS      0x51
#define CMD_RD_DISP_BRIGHTNESS      0x52
#define CMD_WR_CTRL_DISP            0x53
#define CMD_RD_CTRL_DISP            0x54
#define CMD_WR_CABC                 0x55
#define CMD_RD_CABC                 0x56
#define CMD_WR_CABC_MIN             0x5e
#define CMD_RD_CABC_MIN             0x5f
#define CMD_RD_ID1                  0xda
#define CMD_RD_ID2                  0xdb
#define CMD_RD_ID3                  0xdc
// extended
#define CMD_RGB_ITF_SIGNAL_CTRL     0xb0
#define CMD_FRAME_CTRL_NORMAL       0xb1
#define CMD_FRAME_CTRL_IDLE         0xb2
#define CMD_FRAME_CTRL_PARTIAL      0xb3
#define CMD_DISP_INV_CTRL           0xb4
#define CMD_BLANKING_PORCH_CTRL     0xb5
#define CMD_DISP_FUNC_CTRL          0xb6
#define CMD_ENTRY_MODE_SET          0xb7
#define CMD_BACKLIGHT_CTRL_1        0xb8
#define CMD_BACKLIGHT_CTRL_2        0xb9
#define CMD_BACKLIGHT_CTRL_3        0xba
#define CMD_BACKLIGHT_CTRL_4        0xbb
#define CMD_BACKLIGHT_CTRL_5        0xbc
#define CMD_BACKLIGHT_CTRL_7        0xbe
#define CMD_BACKLIGHT_CTRL_8        0xbf
#define CMD_POWER_CTRL_1            0xc0
#define CMD_POWER_CTRL_2            0xc1
#define CMD_VCOM_CTRL_1             0xc5
#define CMD_VCOM_CTRL_2             0xc7
#define CMD_PWR_CTRL_A              0xcb
#define CMD_PWR_CTRL_B              0xcf
#define CMD_NV_MEM_WRITE            0xd0
#define CMD_NV_MEM_PROTECTION_KEY   0xd1
#define CMD_NV_MEM_STATUS_RD        0xd2
#define CMD_RD_ID4                  0xd3
#define CMD_POS_GAMMA_CORRECTION    0xe0
#define CMD_NEG_GAMMA_CORRECTION    0xe1
#define CMD_DIGITAL_GAMMA_CTRL_1    0xe2
#define CMD_DIGITAL_GAMMA_CTRL_2    0xe3
#define CMD_DRIVER_TIMING_CTRL_A    0xe8
#define CMD_DRIVER_TIMING_CTRL_B    0xea
#define CMD_PWR_ON_SEQUENCE_CTRL    0xed
#define CMD_ENABLE_3GAMMA           0xf2
#define CMD_ITF_CTRL                0xf6
#define CMD_PUMP_RATIO_CTRL         0xf7

// memory access control
#define MAC_MY    (1U << 7)
#define MAC_MX    (1U << 6)
#define MAC_MV    (1U << 5)
#define MAC_ML    (1U << 4)
#define MAC_BGR   (1U << 3)
#define MAC_MH    (1U << 2)

// display dimensions
#define ILI9341_TFTWIDTH   240
#define ILI9341_TFTHEIGHT  320

//-----------------------------------------------------------------------------

// execute an operation
// return 0 - operation is complete
// return 1 - wait for spi completion
static int exec_op(struct lcd_drv *drv, struct lcd_op *op) {
	switch (op->type) {
	case LCD_OP_CSASSERT:
		gpio_clr(drv->cfg.cs);
		return 0;
	case LCD_OP_CSDEASSERT:
		gpio_set(drv->cfg.cs);
		return 0;
	case LCD_OP_DATAMODE:
		gpio_set(drv->cfg.dc);	// 1 = data mode
		return 0;
	case LCD_OP_CMD:
		gpio_clr(drv->cfg.dc);	// 0 = command mode
		spi_tx8(drv->cfg.spi, op->u.cmd.cmd, 1);
		return 1;
	case LCD_OP_TXBUF8:
		spi_txbuf8(drv->cfg.spi, op->u.txbuf8.buf, op->u.txbuf8.n);
		return 1;
	case LCD_OP_TX8:
		spi_tx8(drv->cfg.spi, op->u.tx8.data, op->u.tx8.n);
		return 1;
	case LCD_OP_TX16:
		spi_tx16(drv->cfg.spi, op->u.tx16.data, op->u.tx16.n);
		return 1;
	}
	return 0;
}

// flush the operations queue
static void lcd_flush(struct lcd_drv *drv) {
	struct lcd_op_queue *q = &drv->opq;
	while (q->rd != q->wr) {
		struct lcd_op *op = &q->queue[q->rd];
		int wait = exec_op(drv, op);
		if (wait) {
			spi_wait4_done(drv->cfg.spi);
		}
		// advance the read index
		q->rd = (q->rd + 1) & (NUM_OPS - 1);
	}
}

//-----------------------------------------------------------------------------

// enqueue an operation
static int op_enqueue(struct lcd_drv *drv, struct lcd_op *op) {
	struct lcd_op_queue *q = &drv->opq;
	uint32_t saved;
	size_t wr;
	int rc = 0;
	// mask interrupts
	saved = disable_irq();
	wr = (q->wr + 1) & (NUM_OPS - 1);
	if (wr == q->rd) {
		// the queue is full
		rc = -1;
		goto exit;
	}
	// copy the operation data
	q->queue[q->wr] = *op;
	// advance the write index
	q->wr = wr;
 exit:
	// restore interrupts and return
	restore_irq(saved);
	return rc;
}

// assert the chip select
static void lcd_cs_assert(struct lcd_drv *drv) {
	struct lcd_op op;
	op.type = LCD_OP_CSASSERT;
	op_enqueue(drv, &op);
}

// deassert the chip select
static void lcd_cs_deassert(struct lcd_drv *drv) {
	struct lcd_op op;
	op.type = LCD_OP_CSDEASSERT;
	op_enqueue(drv, &op);
}

// send a driver command byte
static void lcd_cmd(struct lcd_drv *drv, uint8_t cmd) {
	struct lcd_op op;
	op.type = LCD_OP_CMD;
	op.u.cmd.cmd = cmd;
	op_enqueue(drv, &op);
	// switch back to data mode
	op.type = LCD_OP_DATAMODE;
	op_enqueue(drv, &op);
}

// Tx a buffer of uint8_t data
// Note: the buffer needs to be valid until this operation is complete.
static void lcd_txbuf8(struct lcd_drv *drv, const uint8_t * buf, size_t n) {
	struct lcd_op op;
	op.type = LCD_OP_TXBUF8;
	op.u.txbuf8.buf = buf;
	op.u.txbuf8.n = n;
	op_enqueue(drv, &op);
}

// Tx n x uint8_t (same value)
static void lcd_tx8(struct lcd_drv *drv, uint8_t data, size_t n) {
	struct lcd_op op;
	op.type = LCD_OP_TX8;
	op.u.tx8.data = data;
	op.u.tx8.n = n;
	op_enqueue(drv, &op);
}

// Tx n x uint16_t (same value)
static void lcd_tx16(struct lcd_drv *drv, uint16_t data, size_t n) {
	struct lcd_op op;
	op.type = LCD_OP_TX16;
	op.u.tx16.data = data;
	op.u.tx16.n = n;
	op_enqueue(drv, &op);
}

//-----------------------------------------------------------------------------

// length, command, command data
static const uint8_t init_table[] = {
	7, CMD_PWR_CTRL_A, 0x39, 0x2c, 0x00, 0x34, 0x02,
	5, CMD_PWR_CTRL_B, 0x00, 0xc1, 0x30,
	5, CMD_DRIVER_TIMING_CTRL_A, 0x85, 0x00, 0x78,
	4, CMD_DRIVER_TIMING_CTRL_B, 0x00, 0x00,
	6, CMD_PWR_ON_SEQUENCE_CTRL, 0x64, 0x03, 0x12, 0x81,
	3, CMD_PUMP_RATIO_CTRL, 0x20,
	3, CMD_POWER_CTRL_1, 0x23,
	3, CMD_POWER_CTRL_2, 0x10,
	4, CMD_VCOM_CTRL_1, 0x3e, 0x28,
	3, CMD_VCOM_CTRL_2, 0x86,
	3, CMD_MEM_ACCESS_CTRL, 0x48,
	4, CMD_VSCROLL_START_ADDR, 0x00, 0x00,
	3, CMD_PIXEL_FMT_SET, 0x55,
	4, CMD_FRAME_CTRL_NORMAL, 0x00, 0x18,
	5, CMD_DISP_FUNC_CTRL, 0x08, 0x82, 0x27,
	3, CMD_ENABLE_3GAMMA, 0x00,
	3, CMD_GAMMA_SET, 0x01,
	17, CMD_POS_GAMMA_CORRECTION, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
	17, CMD_NEG_GAMMA_CORRECTION, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f,
	0,			// end of list
};

// configure the ili9341 chip
static void lcd_configure(struct lcd_drv *drv) {
	const uint8_t *ptr = init_table;
	lcd_cs_assert(drv);
	while (ptr[0] != 0) {
		lcd_cmd(drv, ptr[1]);
		lcd_txbuf8(drv, &ptr[2], ptr[0] - 2);
		ptr += ptr[0];
	}
	lcd_cs_deassert(drv);
}

// turn the led backlight on
static void lcd_backlight_on(struct lcd_drv *drv) {
	gpio_set(drv->cfg.led);
}

// reset the ili9341 chip
static void lcd_reset(struct lcd_drv *drv) {
	gpio_clr(drv->cfg.rst);
	mdelay(10);
	gpio_set(drv->cfg.rst);
	mdelay(50);
}

static void lcd_exit_standby(struct lcd_drv *drv) {
	lcd_cs_assert(drv);
	lcd_cmd(drv, CMD_SLEEP_OUT);
	lcd_flush(drv);
	mdelay(120);
	lcd_cmd(drv, CMD_DISP_ON);
	lcd_cs_deassert(drv);
	lcd_flush(drv);
}

//-----------------------------------------------------------------------------

// set the rotation of the screen
static void lcd_set_rotation(struct lcd_drv *drv, int mode) {
	uint32_t mac;
	switch (mode & 3) {
	case 0:
		mac = (MAC_MX | MAC_BGR);
		drv->width = ILI9341_TFTWIDTH;
		drv->height = ILI9341_TFTHEIGHT;
		break;
	case 1:
		mac = (MAC_MV | MAC_BGR);
		drv->width = ILI9341_TFTHEIGHT;
		drv->height = ILI9341_TFTWIDTH;
		break;
	case 2:
		mac = (MAC_MY | MAC_BGR);
		drv->width = ILI9341_TFTWIDTH;
		drv->height = ILI9341_TFTHEIGHT;
		break;
	case 3:
		mac = (MAC_MX | MAC_MY | MAC_MV | MAC_BGR);
		drv->width = ILI9341_TFTHEIGHT;
		drv->height = ILI9341_TFTWIDTH;
		break;
	}
	lcd_cs_assert(drv);
	lcd_cmd(drv, CMD_MEM_ACCESS_CTRL);
	lcd_tx8(drv, mac, 1);
	lcd_cs_deassert(drv);
	lcd_flush(drv);
}

//-----------------------------------------------------------------------------

// set the region of the graphics ram to write to
static void set_wr_region(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	lcd_cmd(drv, CMD_COLUMN_ADDR_SET);
	lcd_tx16(drv, x, 1);
	lcd_tx16(drv, x + w - 1, 1);
	lcd_cmd(drv, CMD_PAGE_ADDR_SET);
	lcd_tx16(drv, y, 1);
	lcd_tx16(drv, y + h - 1, 1);
	lcd_cmd(drv, CMD_MEM_WR);
}

//-----------------------------------------------------------------------------
// basic graphics operations

// fill a rectangle with a color
void lcd_fill_rect(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	lcd_cs_assert(drv);
	set_wr_region(drv, x, y, w, h);
	lcd_tx16(drv, color, w * h);
	lcd_cs_deassert(drv);
	lcd_flush(drv);
}

// set the scrolling region
void lcd_set_scroll_region(struct lcd_drv *drv, uint16_t tfa, uint16_t vsa) {
	lcd_cs_assert(drv);
	lcd_cmd(drv, CMD_VSCROLL_DEFN);
	lcd_tx16(drv, tfa, 1);
	lcd_tx16(drv, vsa, 1);
	lcd_tx16(drv, ILI9341_TFTHEIGHT - tfa - vsa, 1);	// tfa + vsa + bfa == 320
	lcd_cs_deassert(drv);
	lcd_flush(drv);
}

// set the display scrolling offset
void lcd_scroll(struct lcd_drv *drv, uint16_t vsp) {
	lcd_cs_assert(drv);
	lcd_cmd(drv, CMD_VSCROLL_START_ADDR);
	lcd_tx16(drv, vsp, 1);
	lcd_cs_deassert(drv);
	lcd_flush(drv);
}

// draw a bitmap
void lcd_draw_bitmap(struct lcd_drv *drv, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t bg, const uint32_t * buf) {
#if 0
	lcd_cs_assert(drv);
	set_wr_region(drv, x, y, w, h);
	uint32_t count = w * h;
	int i = 0;
	while (count) {
		uint32_t bitmap = buf[i];
		uint32_t mask = 1U << 31;
		uint32_t n = (count > 32) ? 32 : count;
		for (uint32_t j = 0; j < n; j++) {
			spi_tx16(drv->cfg.spi, (bitmap & mask) ? color : bg);
			mask >>= 1;
		}
		count -= n;
		i += 1;
	}
	lcd_cs_deassert(drv);
#endif
}

//-----------------------------------------------------------------------------

// initialise the lcd driver chip
int lcd_init(struct lcd_drv *drv, struct lcd_cfg *cfg) {
	memset(drv, 0, sizeof(struct lcd_drv));
	drv->cfg = *cfg;

	// lcd driver init
	lcd_reset(drv);
	lcd_backlight_on(drv);
	lcd_configure(drv);
	lcd_exit_standby(drv);

	// screen setup
	lcd_set_rotation(drv, drv->cfg.rotation);
	lcd_fill_rect(drv, 0, 0, drv->width, drv->height, drv->cfg.bg);

	return 0;
}

//-----------------------------------------------------------------------------
