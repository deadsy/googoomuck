//-----------------------------------------------------------------------------
/*

Audio Control for the STM32F4 Discovery Board

*/
//-----------------------------------------------------------------------------

#ifndef AUDIO_H
#define AUDIO_H

//-----------------------------------------------------------------------------

#include "stm32f4_soc.h"
#include "cs43l22.h"
#include "utils.h"

//-----------------------------------------------------------------------------

// Total bits/sec = AUDIO_SAMPLE_RATE * N bits per channel * 2 channels
// This is used to lookup the i2s clock configuration in a table.
#define AUDIO_SAMPLE_RATE 44100U	// Hz

// The hardware is often not capable of the exact sample rate.
// This is the sample rate we actually get per the clock divider settings.
// See ./scripts/i2sclk.py for details.
#define AUDIO_FS 44099.507f	// Hz

// The size (in audio samples) of the work buffer.
#define AUDIO_BLOCK_SIZE 128

// The size (in audio samples) of the buffer that is DMAed from memory to I2S.
#define AUDIO_BUFFER_SIZE (4 * AUDIO_BLOCK_SIZE)
#define HALF_AUDIO_BUFFER_SIZE (2 * AUDIO_BLOCK_SIZE)

//-----------------------------------------------------------------------------

#define N_MARGINS 16U		// must be a power of 2

struct audio_stats {
	uint32_t buffers;
	uint32_t underrun;	// number of DMA buffer underruns
	int max;		// maximum margin (in audio samples)
	int min;		// minimum margin (in audio samples)
	int margins[N_MARGINS];	// storage for moving average
	int idx;		// storage write index
};

struct audio_drv {
	struct dma_drv dma;
	struct i2s_drv i2s;
	struct i2c_drv i2c;
	struct cs4x_drv dac;
	struct audio_stats stats;
	int16_t buffer[AUDIO_BUFFER_SIZE] ALIGN(4);	// dma->i2s buffer
};

extern struct audio_drv ggm_audio;

//-----------------------------------------------------------------------------

int audio_init(struct audio_drv *audio);
int audio_start(struct audio_drv *audio);
void audio_wr(int16_t * dst, size_t n, float *ch_l, float *ch_r);
void audio_stats(struct audio_drv *audio, int16_t * buf);
void audio_master_volume(struct audio_drv *audio, uint8_t vol);

//-----------------------------------------------------------------------------

#endif				// AUDIO_H

//-----------------------------------------------------------------------------
