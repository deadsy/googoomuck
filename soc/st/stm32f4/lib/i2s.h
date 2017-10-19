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
#define I2S_MODE_SLAVE_TX (0 << 8)
#define I2S_MODE_SLAVE_RX (1 << 8)
#define I2S_MODE_MASTER_TX (2 << 8)
#define I2S_MODE_MASTER_RX (3 << 8)

// i2s standard (I2SSTD, PCMSYNC)
#define I2S_STANDARD_PHILIPS (0 << 4)
#define I2S_STANDARD_MSB (1 << 4)
#define I2S_STANDARD_LSB (2 << 4)
#define I2S_STANDARD_PCM_SHORT ((3 << 4) | (0 << 7))
#define I2S_STANDARD_PCM_LONG ((3 << 4) | (1 << 7))

// data format (DATLEN, CHLEN)
#define I2S_DATAFORMAT_16B ((0 << 1 /*datlen=16*/) | (0 << 0 /*chlen = 16*/))
#define I2S_DATAFORMAT_16B_EXTENDED ((0 << 1 /*datlen=16*/) | (1 << 0 /*chlen=32*/))
#define I2S_DATAFORMAT_24B ((1 << 1 /*datlen=24*/) | (1 << 1 /*chlen=32*/))
#define I2S_DATAFORMAT_32B ((2 << 1 /*datlen=32*/) | (1 << 1 /*chlen=32*/))

// clock polarity (CKPOL)
#define I2S_CPOL_LOW (0 << 3)
#define I2S_CPOL_HIGH (1 << 3)

//-----------------------------------------------------------------------------

struct i2s_cfg {
	int idx;		// i2s device to use
	uint32_t mode;		// operating mode
	uint32_t standard;	// standard used
	uint32_t data_format;	// data format
	int mclk_output;	// is mclk output enabled?
	int audio_freq;		// frequency
	uint32_t clk_polarity;	// clock polarity
	int clk_src;		// clock source
	int fdx_mode;		// is full duplex mode enabled?
};

struct i2s_drv {
	struct i2s_cfg cfg;	// configuration values
	SPI_TypeDef *base;	// base address of SPI/I2S peripheral

};

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg);

//-----------------------------------------------------------------------------

#endif				// I2S_H

//-----------------------------------------------------------------------------
