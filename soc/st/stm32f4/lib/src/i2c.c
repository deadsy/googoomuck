//-----------------------------------------------------------------------------
/*

I2C Bit-Bang Driver

*/
//-----------------------------------------------------------------------------

#include "i2c.h"
#include "gpio.h"
#include "delay.h"

//-----------------------------------------------------------------------------
// Low level SCL/SDA routines

// set high impedance of the SCL line
static inline void scl_rel(struct i2cbus *bus) {
	gpio_dirn_in(bus->scl);
}

// set high impedance of the SDA line
static inline void sda_rel(struct i2cbus *bus) {
	gpio_dirn_in(bus->sda);
}

// drive SCL line low
static inline void scl_lo(struct i2cbus *bus) {
	gpio_dirn_out(bus->scl);
}

// drive SDA line low
static inline void sda_lo(struct i2cbus *bus) {
	gpio_dirn_out(bus->sda);
}

// read the SCL line
static inline int scl_rd(struct i2cbus *bus) {
	return gpio_rd(bus->scl);
}

// read the SDA line
static inline int sda_rd(struct i2cbus *bus) {
	return gpio_rd(bus->sda);
}

// relatively slow clocking ~10kHz.
static void i2c_delay(void) {
	udelay(100);
}

//-----------------------------------------------------------------------------

// Initialise the i2c bus.
int i2c_init(struct i2cbus *bus, uint16_t scl, uint16_t sda) {
	bus->scl = scl;
	bus->sda = sda;
	gpio_dirn_in(bus->scl);
	gpio_dirn_in(bus->sda);
	gpio_clr(bus->scl);
	gpio_clr(bus->sda);
	return I2C_OK;
}

// return non-zero if the device acks
int i2c_scan(struct i2cbus *bus, uint8_t adr) {
	uint8_t buf[1];
	return i2c_rd_buf(bus, adr, buf, sizeof(buf)) == I2C_OK;
}

//-----------------------------------------------------------------------------

// Create start condition- SDA goes low while SCL is high.
// On Exit- SDA and SCL are held low.
static int i2c_start(struct i2cbus *bus) {
	// release the clock and data lines
	scl_rel(bus);
	sda_rel(bus);
	// check that scl and sda are both high (no bus contention)
	i2c_delay();
	if (!sda_rd(bus) || !scl_rd(bus)) {
		return I2C_ERR_BUS;
	}
	sda_lo(bus);
	i2c_delay();
	scl_lo(bus);
	i2c_delay();
	return I2C_OK;
}

// Create stop condition- SDA goes high while SCL is high.
// On Exit- SDA and SCL are released.
static void i2c_stop(struct i2cbus *bus) {
	scl_lo(bus);
	i2c_delay();
	sda_lo(bus);
	i2c_delay();
	scl_rel(bus);
	i2c_delay();
	sda_rel(bus);
	i2c_delay();
}

#define I2C_SLAVE_DELAY 100

// Clock SCL and read SDA at clock high.
// On Entry- SCL is held low.
// On Exit- SCL is held low, SDA =0/1 is returned.
static int i2c_clock(struct i2cbus *bus) {
	int delay = I2C_SLAVE_DELAY;
	i2c_delay();
	scl_rel(bus);
	// wait for any slave clock stretching
	while (!scl_rd(bus) && delay > 0) {
		udelay(100);
		delay--;
	}
	if (delay == 0) {
		i2c_stop(bus);
		return I2C_ERR_SLV;
	}
	// read the data
	i2c_delay();
	int val = sda_rd(bus);
	scl_lo(bus);
	i2c_delay();
	return val;
}

// Write a byte of data to the slave.
// On Entry- SCL is held low.
// On Exit- SDA is released, SCL is held low.
static void i2c_wr_byte(struct i2cbus *bus, uint8_t val) {
	uint8_t mask = 0x80;
	while (mask != 0) {
		if (val & mask) {
			sda_rel(bus);
		} else {
			sda_lo(bus);
		}
		i2c_clock(bus);
		mask >>= 1;
	}
	sda_rel(bus);
}

// Read a byte from a slave.
// On Entry- SCL is held low.
// On Exit- SDA is released, SCL is held low
static uint8_t i2c_rd_byte(struct i2cbus *bus) {
	uint8_t val = 0;
	int i;
	sda_rel(bus);
	for (i = 0; i < 8; i++) {
		val <<= 1;
		val |= i2c_clock(bus);
	}
	return val;
}

// Send an ack to the slave.
static void i2c_wr_ack(struct i2cbus *bus) {
	sda_lo(bus);
	i2c_clock(bus);
	sda_rel(bus);
}

// Clock in the SDA level from the slave.
// Return: 0 = no ack, 1 = ack
static int i2c_rd_ack(struct i2cbus *bus) {
	sda_rel(bus);
	return i2c_clock(bus) == 0;
}

// Read n bytes from device adr
int i2c_rd_buf(struct i2cbus *bus, uint8_t adr, uint8_t * buf, size_t n) {
	unsigned int i;
	// start a read cycle
	if (i2c_start(bus) < 0) {
		return I2C_ERR_BUS;
	}
	// address the device
	i2c_wr_byte(bus, adr | 1);
	if (!i2c_rd_ack(bus)) {
		i2c_stop(bus);
		return I2C_ERR_ADR;
	}
	// read data
	for (i = 0; i < n; i++) {
		buf[i] = i2c_rd_byte(bus);
		// The last byte from the slave is not acked
		if (i < n - 1) {
			i2c_wr_ack(bus);
		}
	}
	i2c_stop(bus);
	return I2C_OK;
}

// Write a buffer of bytes to device adr
int i2c_wr_buf(struct i2cbus *bus, uint8_t adr, uint8_t * buf, size_t n) {
	unsigned int i;
	// start a write cycle
	if (i2c_start(bus) < 0) {
		return I2C_ERR_BUS;
	}
	// address the device
	i2c_wr_byte(bus, adr & ~1);
	if (!i2c_rd_ack(bus)) {
		i2c_stop(bus);
		return I2C_ERR_ADR;
	}
	// write data
	for (i = 0; i < n; i++) {
		i2c_wr_byte(bus, buf[i]);
		if (!i2c_rd_ack(bus)) {
			// no ack from slave
			i2c_stop(bus);
			return I2C_ERR_NAK;
		}
	}
	i2c_stop(bus);
	return I2C_OK;
}

//-----------------------------------------------------------------------------
