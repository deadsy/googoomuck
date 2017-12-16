//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ili9341.h"

//-----------------------------------------------------------------------------
// Command Set

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

//-----------------------------------------------------------------------------
int ili9341_init(struct ili9341_drv *lcd, struct ili9341_cfg *cfg) {
	memset(lcd, 0, sizeof(struct ili9341_drv));
	return 0;
}

//-----------------------------------------------------------------------------
