//-----------------------------------------------------------------------------
/*

I2C Bit-Bang Driver

*/
//-----------------------------------------------------------------------------

#include "i2c.h"
#include "gpio.h"
#include "delay.h"

#include "target.h"

//-----------------------------------------------------------------------------
// Low level SCL/SDA routines

// set high impedance of the SCL line
static inline void scl_rel(void) {
	gpio_dirn_in(GPIO_I2C_SCL);
}

// set high impedance of the SDA line
static inline void sda_rel(void) {
	gpio_dirn_in(GPIO_I2C_SDA);
}

// drive SCL line low
static inline void scl_lo(void) {
	gpio_dirn_out(GPIO_I2C_SCL);
}

// drive SDA line low
static inline void sda_lo(void) {
	gpio_dirn_out(GPIO_I2C_SDA);
}

// read the SCL line
static inline int scl_rd(void) {
	return gpio_rd(GPIO_I2C_SCL);
}

// read the SDA line
static inline int sda_rd(void) {
	return gpio_rd(GPIO_I2C_SDA);
}

void i2c_init(void) {
	gpio_dirn_in(GPIO_I2C_SCL);
	gpio_dirn_in(GPIO_I2C_SDA);
	gpio_clr(GPIO_I2C_SCL);
	gpio_clr(GPIO_I2C_SDA);
}

// relatively slow clocking ~10kHz.
static void i2c_delay(void) {
	udelay(100);
}

//-----------------------------------------------------------------------------
// I2C Bit Banging Driver

// Create start condition- SDA goes low while SCL is high.
// On Exit- SDA and SCL are held low.
static int i2c_start(void) {
	// release the clock and data lines
	scl_rel();
	sda_rel();
	// check that scl and sda are both high (no bus contention)
	i2c_delay();
	if (!sda_rd() || !scl_rd()) {
		return I2C_ERR_BUS;
	}
	sda_lo();
	i2c_delay();
	scl_lo();
	i2c_delay();
	return I2C_OK;
}

// Create stop condition- SDA goes high while SCL is high.
// On Exit- SDA and SCL are released.
static void i2c_stop(void) {
	scl_lo();
	i2c_delay();
	sda_lo();
	i2c_delay();
	scl_rel();
	i2c_delay();
	sda_rel();
	i2c_delay();
}

#define I2C_SLAVE_DELAY 100

// Clock SCL and read SDA at clock high.
// On Entry- SCL is held low.
// On Exit- SCL is held low, SDA =0/1 is returned.
static int i2c_clock(void) {
	int delay = I2C_SLAVE_DELAY;
	i2c_delay();
	scl_rel();
	// wait for any slave clock stretching
	while (!scl_rd() && delay > 0) {
		udelay(100);
		delay--;
	}
	if (delay == 0) {
		i2c_stop();
		return I2C_ERR_SLV;
	}
	// read the data
	i2c_delay();
	int val = sda_rd();
	scl_lo();
	i2c_delay();
	return val;
}

// Write a byte of data to the slave.
// On Entry- SCL is held low.
// On Exit- SDA is released, SCL is held low.
static void i2c_wr_byte(uint8_t val) {
	uint8_t mask = 0x80;
	while (mask != 0) {
		if (val & mask) {
			sda_rel();
		} else {
			sda_lo();
		}
		i2c_clock();
		mask >>= 1;
	}
	sda_rel();
}

// Read a byte from a slave.
// On Entry- SCL is held low.
// On Exit- SDA is released, SCL is held low
static uint8_t i2c_rd_byte(void) {
	uint8_t val = 0;
	int i;
	sda_rel();
	for (i = 0; i < 8; i++) {
		val <<= 1;
		val |= i2c_clock();
	}
	return val;
}

// Send an ack to the slave.
static void i2c_wr_ack(void) {
	sda_lo();
	i2c_clock();
	sda_rel();
}

// Clock in the SDA level from the slave.
// Return: 0 = no ack, 1 = ack
static int i2c_rd_ack(void) {
	sda_rel();
	return i2c_clock() == 0;
}

// Read n bytes from device adr
int i2c_rd_buf(uint8_t adr, uint8_t * buf, size_t n) {
	unsigned int i;
	// start a read cycle
	if (i2c_start() < 0) {
		return I2C_ERR_BUS;
	}
	// address the device
	i2c_wr_byte(adr | 1);
	if (!i2c_rd_ack()) {
		i2c_stop();
		return I2C_ERR_ADR;
	}
	// read data
	for (i = 0; i < n; i++) {
		buf[i] = i2c_rd_byte();
		// The last byte from the slave is not acked
		if (i < n - 1) {
			i2c_wr_ack();
		}
	}
	i2c_stop();
	return I2C_OK;
}

// Write a buffer of bytes to device adr
int i2c_wr_buf(uint8_t adr, uint8_t * buf, size_t n) {
	unsigned int i;
	// start a write cycle
	if (i2c_start() < 0) {
		return I2C_ERR_BUS;
	}
	// address the device
	i2c_wr_byte(adr & ~1);
	if (!i2c_rd_ack()) {
		i2c_stop();
		return I2C_ERR_ADR;
	}
	// write data
	for (i = 0; i < n; i++) {
		i2c_wr_byte(buf[i]);
		if (!i2c_rd_ack()) {
			// no ack from slave
			i2c_stop();
			return I2C_ERR_NAK;
		}
	}
	i2c_stop();
	return I2C_OK;
}

//-----------------------------------------------------------------------------
