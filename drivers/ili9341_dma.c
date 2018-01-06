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

#define LCD_OP_FLAG_CS_ASSERT (1U << 0)	// assert the chip select line
#define LCD_OP_FLAG_CS_DEASSERT (1U << 1)	// deassert the chip select line
#define LCD_OP_FLAG_DC_ASSERT (1U << 2)	// assert the d/c line
#define LCD_OP_FLAG_DC_DEASSERT (1U << 3)	// deassert the d/c line

#define LCD_OP_TYPE_NONE (0U << 4)	// no data transfer
#define LCD_OP_TYPE_8BIT (0U << 4)	// no data transfer
#define LCD_OP_TYPE_16BIT (0U << 4)	// no data transfer

#define LCD_OP_TYPE_BUF8 (1U << 4)	// 8 bit data, buffer in operation
#define LCD_OP_TYPE_PTR8 (2U << 4)	// 8 bit data, pointer in operation
#define LCD_OP_TYPE_BUF16 (3U << 4)	// 16 bit data, buffer in operation
#define LCD_OP_TYPE_PTR16 (4U << 4)	// 16 bit data, pointer in operation

struct lcd_operation {
	uint32_t flags;

};

//-----------------------------------------------------------------------------
/*

Notes on an interrupt/dma driven lcd driver.

types of queued operations:

command:

command byte
data bytes
transaction state

cmd, ptr to data, length of data
cmd, small data buffer, length of data

cs assert (flag)
cs deassert (flag)
d/c assert (flag)
d/c deassert (flag)
data size (8/16)
data to send (ptr or buffer)

#define LCD_FLAG_CS_ASSERT
#define LCD_FLAG_CS_DEASSERT
#define LCD_FLAG_DC_ASSERT
#define LCD_FLAG_DC_DEASSERT

#define LCD_OP_TYPE_PTR
#define LCD_OP_TYPE_BUF

#DEFINE LCD_OP_DATA8
#DEFINE LCD_OP_DATA16


*/
//-----------------------------------------------------------------------------

struct lcd_op {
  uint8_t type; // type of lcd operation
  uint8_t state; // state of operation
};

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

// send a command byte to the driver chip
static int exec_op_cmd(struct lcd_drv *drv, struct lcd_op *op) {
  if (op->state == 0) {
    // start of command
    gpio_clr(drv->cfg.dc);	// 0 = command mode
    spi_dma_tx8(drv->cfg.spi, op->cmd);
    op->state = 1;
    // not done
    return 0;
  }
  // end of command
  gpio_set(drv->cfg.dc);	// 1 = data mode
  // done
  return 1;
}

// send a buffer of bytes to the driver chip
static int exec_op_tx8(struct lcd_drv *drv, struct lcd_op *op) {
	spi_dma_txbuf8(drv->cfg.spi, op->buf, op->n);
  return 1;
}

static int exec_op_cs_assert(struct lcd_drv *drv, struct lcd_op *op) {
  gpio_clr(drv->cfg.cs);
  return 1;
}

static int exec_op_cs_deassert(struct lcd_drv *drv, struct lcd_op *op) {
  gpio_set(drv->cfg.cs);
  return 1;
}

//-----------------------------------------------------------------------------

static void queue_op_cmd(struct lcd_drv *drv, uint8_t cmd) {
  struct lcd_op *op = get_op(drv);
  op->type = LCD_OP_CMD;
  op->state = 0;
}






// queue a command to the driver chip
static void queue_command(struct lcd_drv *drv, uint8_t cmd, uint8_t *buf, size_t n) {

  // queue the command operation

  // queue the data operation




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
		wr_cmd(drv, ptr[1]);
		spi_txbuf8(drv->cfg.spi, &ptr[2], ptr[0] - 2);
		ptr += ptr[0];
	}
	lcd_cs_deassert(drv);
}

//-----------------------------------------------------------------------------

int lcd_init(struct lcd_drv *drv, struct lcd_cfg *cfg) {
	memset(drv, 0, sizeof(struct lcd_drv));
	drv->cfg = *cfg;

	// lcd driver init
	lcd_reset(drv);
	lcd_backlight_on(drv);
	lcd_configure(drv);
	//lcd_exit_standby(drv);

	// screen setup
	//lcd_set_rotation(drv, drv->cfg.rotation);
	//lcd_fill_rect(drv, 0, 0, drv->width, drv->height, drv->cfg.bg);

	return 0;
}

//-----------------------------------------------------------------------------
