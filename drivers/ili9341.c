//-----------------------------------------------------------------------------
/*

ILI9341 LCD Driver

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ili9341.h"

//-----------------------------------------------------------------------------
// Command Set

#define ILI9341_CMD_NOP        0x00
#define ILI9341_CMD_SWRESET    0x01
#define ILI9341_CMD_RDDID      0x04
#define ILI9341_CMD_RDDST      0x09
#define ILI9341_CMD_RDMODE     0x0A
#define ILI9341_CMD_RDMADCTL   0x0B
#define ILI9341_CMD_RDPIXFMT   0x0C
#define ILI9341_CMD_RDIMGFMT   0x0D
#define ILI9341_CMD_RDSELFDIAG 0x0F
#define ILI9341_CMD_SLPIN      0x10
#define ILI9341_CMD_SLPOUT     0x11
#define ILI9341_CMD_PTLON      0x12
#define ILI9341_CMD_NORON      0x13
#define ILI9341_CMD_INVOFF     0x20
#define ILI9341_CMD_INVON      0x21
#define ILI9341_CMD_GAMMASET   0x26
#define ILI9341_CMD_DISPOFF    0x28
#define ILI9341_CMD_DISPON     0x29
#define ILI9341_CMD_CASET      0x2A
#define ILI9341_CMD_PASET      0x2B
#define ILI9341_CMD_RAMWR      0x2C
#define ILI9341_CMD_RAMRD      0x2E
#define ILI9341_CMD_PTLAR      0x30
#define ILI9341_CMD_MADCTL     0x36
#define ILI9341_CMD_VSCRSADD   0x37
#define ILI9341_CMD_PIXFMT     0x3A
#define ILI9341_CMD_FRMCTR1    0xB1
#define ILI9341_CMD_FRMCTR2    0xB2
#define ILI9341_CMD_FRMCTR3    0xB3
#define ILI9341_CMD_INVCTR     0xB4
#define ILI9341_CMD_DFUNCTR    0xB6
#define ILI9341_CMD_PWCTR1     0xC0
#define ILI9341_CMD_PWCTR2     0xC1
#define ILI9341_CMD_PWCTR3     0xC2
#define ILI9341_CMD_PWCTR4     0xC3
#define ILI9341_CMD_PWCTR5     0xC4
#define ILI9341_CMD_VMCTR1     0xC5
#define ILI9341_CMD_VMCTR2     0xC7
#define ILI9341_CMD_RDID1      0xDA
#define ILI9341_CMD_RDID2      0xDB
#define ILI9341_CMD_RDID3      0xDC
#define ILI9341_CMD_RDID4      0xDD
#define ILI9341_CMD_GMCTRP1    0xE0
#define ILI9341_CMD_GMCTRN1    0xE1
#define ILI9341_CMD_PWCTR6     0xFC

//-----------------------------------------------------------------------------

int ili9341_init(struct ili9341_drv *lcd, struct ili9341_cfg *cfg) {
	memset(lcd, 0, sizeof(struct ili9341_drv));
	return 0;
}

//-----------------------------------------------------------------------------
