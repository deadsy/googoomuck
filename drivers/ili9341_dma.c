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
