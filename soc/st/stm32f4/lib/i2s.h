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

// audio frequency
#define I2S_AUDIOFREQ_192000 (192000U)
#define I2S_AUDIOFREQ_96000 (96000U)
#define I2S_AUDIOFREQ_48000 (48000U)
#define I2S_AUDIOFREQ_44100 (44100U)
#define I2S_AUDIOFREQ_32000 (32000U)
#define I2S_AUDIOFREQ_22050 (22050U)
#define I2S_AUDIOFREQ_16000 (16000U)
#define I2S_AUDIOFREQ_11025 (11025U)
#define I2S_AUDIOFREQ_8000 (8000U)

// master clock output enable
#define I2S_MCLKOUTPUT_ENABLE (1U << 9)
#define I2S_MCLKOUTPUT_DISABLE (0U << 9)

//-----------------------------------------------------------------------------

struct i2s_cfg {
	int idx;		// i2s device to use
	uint32_t mode;		// operating mode
	uint32_t standard;	// standard used
	uint32_t data_format;	// data format
	uint32_t mclk_output;	// is mclk output enabled?
	uint32_t audio_freq;	// frequency
	uint32_t clk_polarity;	// clock polarity
	int fdx_mode;		// is full duplex mode enabled?
};

struct i2s_drv {
	struct i2s_cfg cfg;	// configuration values
	SPI_TypeDef *base;	// base address of SPI/I2S peripheral

};

//-----------------------------------------------------------------------------

int i2s_init(struct i2s_drv *i2s, struct i2s_cfg *cfg);
int i2s_clock(uint32_t fs);

//-----------------------------------------------------------------------------

#endif				// I2S_H

//-----------------------------------------------------------------------------
