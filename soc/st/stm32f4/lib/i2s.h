//-----------------------------------------------------------------------------
/*

I2S Driver

*/
//-----------------------------------------------------------------------------

#ifndef I2S_H
#define I2S_H

//-----------------------------------------------------------------------------

#ifndef STM32F4_SOC_H
#warning "please include this file using the toplevel stm32f4_soc.h"
#endif

//-----------------------------------------------------------------------------

// operating modes (I2SCFG)
#define I2S_MODE_SLAVE_TX (0U << 8)
#define I2S_MODE_SLAVE_RX (1U << 8)
#define I2S_MODE_MASTER_TX (2U << 8)
#define I2S_MODE_MASTER_RX (3U << 8)

// i2s standard (I2SSTD, PCMSYNC)
#define I2S_STANDARD_PHILIPS (0U << 4)
#define I2S_STANDARD_MSB (1U << 4)
#define I2S_STANDARD_LSB (2U << 4)
#define I2S_STANDARD_PCM_SHORT ((3U << 4) | (0U << 7))
#define I2S_STANDARD_PCM_LONG ((3U << 4) | (1U << 7))

// data format (DATLEN, CHLEN)
#define I2S_DATAFORMAT_16B ((0U << 1) | (0U << 0))
#define I2S_DATAFORMAT_16B_EXTENDED ((0U << 1) | (1U << 0))
#define I2S_DATAFORMAT_24B ((1U << 1) | (1U << 1))
#define I2S_DATAFORMAT_32B ((2U << 1) | (1U << 1))

// clock polarity (CKPOL)
#define I2S_CPOL_LOW (0U << 3)
#define I2S_CPOL_HIGH (1U << 3)

// master clock output enable
#define I2S_MCLKOUTPUT_ENABLE (1U << 9)
#define I2S_MCLKOUTPUT_DISABLE (0U << 9)

//-----------------------------------------------------------------------------

struct i2s_cfg {
	int idx;		// i2s device to use
	uint32_t mode;		// operating mode
	uint32_t standard;	// standard used
	uint32_t data_format;	// data format
	uint32_t mckoe;		// is the master clock output enabled?
	uint32_t cpol;		// clock polarity
	uint32_t fs;		// audio sample rate
};

struct i2s_drv {
	struct i2s_cfg cfg;	// configuration values
	SPI_TypeDef *regs;	// SPI/I2S peripheral registers
};

//-----------------------------------------------------------------------------

static inline void i2s_enable(struct i2s_drv *i2s) {
	i2s->regs->I2SCFGR |= (1 << 10);	// I2SE
}

static inline void i2s_disable(struct i2s_drv *i2s) {
	i2s->regs->I2SCFGR &= ~(1 << 10);	// I2SE
}

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg);
int i2s_wr(struct i2s_drv *i2s, int16_t val);
uint32_t i2s_get_fsclk(struct i2s_drv *i2s);
uint32_t i2s_get_DR(struct i2s_drv *i2s);

int set_i2sclk(uint32_t fs);
uint32_t get_i2sclk(void);

//-----------------------------------------------------------------------------

#endif				// I2S_H

//-----------------------------------------------------------------------------
