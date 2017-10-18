//-----------------------------------------------------------------------------
/*

I2C Bit-Bang Driver

*/
//-----------------------------------------------------------------------------

#ifndef I2C_H
#define I2C_H

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

//-----------------------------------------------------------------------------

struct i2c_bus {
	int scl;
	int sda;
	int delay;
};

//-----------------------------------------------------------------------------

#define I2C_OK 0		// ok
#define I2C_ERR_BUS -1		// bus error
#define I2C_ERR_ADR -2		// no response
#define I2C_ERR_NAK -3		// missing ack
#define I2C_ERR_SLV -4		// no slave data

//-----------------------------------------------------------------------------

int i2c_init(struct i2c_bus *bus, int scl, int sda, int delay);
int i2c_wr_buf(struct i2c_bus *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_rd_buf(struct i2c_bus *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_scan(struct i2c_bus *bus, uint8_t adr);

//-----------------------------------------------------------------------------

#endif				// I2C_H

//-----------------------------------------------------------------------------
