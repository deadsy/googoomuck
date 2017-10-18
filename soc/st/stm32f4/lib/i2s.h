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

struct i2s_cfg {
	int mode;		// operating mode
	int standard;		// standard used
	int data_format;	// data format
	int mclk_output;	// is mclk output enabled?
	int audio_freq;		// frequency
	int clk_polarity;	// clock polarity
	int clk_src;		// clock source
	int fdx_mode;		// is full duplex mode enabled?
};

struct i2s_drv {
	SPI_TypeDef *base;	// base address of SPI/I2S peripheral
	struct i2s_cfg cfg;	// configuration values
};

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg, SPI_TypeDef * base);

//-----------------------------------------------------------------------------

#endif				// I2S_H

//-----------------------------------------------------------------------------
