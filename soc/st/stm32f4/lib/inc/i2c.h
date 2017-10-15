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

struct i2cbus {
	uint16_t scl;
	uint16_t sda;
};

//-----------------------------------------------------------------------------

#define I2C_OK 0		// ok
#define I2C_ERR_BUS -1		// bus error
#define I2C_ERR_ADR -2		// no response
#define I2C_ERR_NAK -3		// missing ack
#define I2C_ERR_SLV -4		// no slave data

//-----------------------------------------------------------------------------

int i2c_init(struct i2cbus *bus, uint16_t scl, uint16_t sda);
int i2c_wr_buf(struct i2cbus *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_rd_buf(struct i2cbus *bus, uint8_t adr, uint8_t * buf, size_t n);
int i2c_scan(struct i2cbus *bus, uint8_t adr);

//-----------------------------------------------------------------------------

#endif				// I2C_H

//-----------------------------------------------------------------------------
