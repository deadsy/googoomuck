//-----------------------------------------------------------------------------
/*

I2C Bit-Bang Driver

*/
//-----------------------------------------------------------------------------

#ifndef I2C_H
#define I2C_H

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

//-----------------------------------------------------------------------------

struct i2c_cfg {
	int scl;
	int sda;
	int delay;
};

struct i2c_drv {
	struct i2c_cfg cfg;
};

//-----------------------------------------------------------------------------

#define I2C_OK 0		// ok
#define I2C_ERR_BUS -1		// bus error
#define I2C_ERR_ADR -2		// no response
#define I2C_ERR_NAK -3		// missing ack
#define I2C_ERR_SLV -4		// no slave data

//-----------------------------------------------------------------------------

int i2c_init(struct i2c_drv *bus, struct i2c_cfg *cfg);
int i2c_wr(struct i2c_drv *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_rd(struct i2c_drv *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_scan(struct i2c_drv *bus, uint8_t adr);

//-----------------------------------------------------------------------------

#endif				// I2C_H

//-----------------------------------------------------------------------------
