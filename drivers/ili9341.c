//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

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
#define CMD_NV_MEM_WRITE            0xd0
#define CMD_NV_MEM_PROTECTION_KEY   0xd1
#define CMD_NV_MEM_STATUS_RD        0xd2
#define CMD_RD_ID4                  0xd3
#define CMD_POS_GAMMA_CORRECTION    0xe0
#define CMD_NEG_GAMMA_CORRECTION    0xe1
#define CMD_DIGITAL_GAMMA_CTRL_1    0xe2
#define CMD_DIGITAL_GAMMA_CTRL_2    0xe3
#define CMD_ITF_CTRL                0xf6

//-----------------------------------------------------------------------------

static void wr_cmd(struct ili9341_drv *drv, uint8_t cmd) {
	// TODO set command/data selector
	spi_wr_block(drv->cfg.spi, cmd);
}

static void wr_data8(struct ili9341_drv *drv, uint8_t data) {
	spi_wr_block(drv->cfg.spi, data);
}

#if 0
static void wr_data16(struct ili9341_drv *drv, uint16_t data) {
	wr_data8(drv, data >> 8);
	wr_data8(drv, data);
}

static void wr_data32(struct ili9341_drv *drv, uint32_t data) {
	wr_data16(drv, data >> 16);
	wr_data16(drv, data);
}
#endif

//-----------------------------------------------------------------------------

// length, command, command_data
static const uint8_t init_table[] = {
	5, 0xef, 0x03, 0x80, 0x02,
	5, 0xcf, 0x00, 0xc1, 0x30,
	6, 0xed, 0x64, 0x03, 0x12, 0x81,
	5, 0xe8, 0x85, 0x00, 0x78,
	7, 0xcb, 0x39, 0x2c, 0x00, 0x34, 0x02,
	3, 0xf7, 0x20,
	4, 0xea, 0x00, 0x00,
	3, CMD_POWER_CTRL_1, 0x23,
	3, CMD_POWER_CTRL_2, 0x10,
	4, CMD_VCOM_CTRL_1, 0x3e, 0x28,
	3, CMD_VCOM_CTRL_2, 0x86,
	3, CMD_MEM_ACCESS_CTRL, 0x48,
	4, CMD_VSCROLL_START_ADDR, 0x00, 0x00,
	3, CMD_PIXEL_FMT_SET, 0x55,
	4, CMD_FRAME_CTRL_NORMAL, 0x00, 0x18,
	5, CMD_DISP_FUNC_CTRL, 0x08, 0x82, 0x27,
	3, 0xf2, 0x00,		// 3Gamma Function Disable
	3, CMD_GAMMA_SET, 0x01,
	17, CMD_POS_GAMMA_CORRECTION, 0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1, 0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
	17, CMD_NEG_GAMMA_CORRECTION, 0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1, 0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f,
	0,			// end of list
};

int ili9341_init(struct ili9341_drv *drv, struct ili9341_cfg *cfg) {
	memset(drv, 0, sizeof(struct ili9341_drv));
	drv->cfg = *cfg;

	// apply the initialisation table commands
	const uint8_t *ptr = init_table;
	while (ptr[0] != 0) {
		wr_cmd(drv, ptr[1]);
		for (int i = 0; i < ptr[0] - 2; i++) {
			wr_data8(drv, ptr[2 + i]);
		}
		ptr += ptr[0];
	}

	return 0;
}

//-----------------------------------------------------------------------------
