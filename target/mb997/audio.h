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

//-----------------------------------------------------------------------------

// Total bits/sec = AUDIO_SAMPLE_RATE * N bits per channel * 2 channels
// This is used to lookup the i2s clock configuration in a table.
#define AUDIO_SAMPLE_RATE 35156U	// Hz

// The hardware is often not capable of the exact sample rate.
// This is the sample rate we actually get per the clock divider settings.
// See ./scripts/i2sclk.py for details.
#define AUDIO_FS 35156.25f	// Hz

//-----------------------------------------------------------------------------

struct audio_drv {
	struct dma_drv dma;
	struct i2s_drv i2s;
	struct i2c_drv i2c;
	struct cs4x_drv dac;
};

//-----------------------------------------------------------------------------

int audio_init(struct audio_drv *audio);
int audio_start(struct audio_drv *audio);
void audio_wr(struct audio_drv *audio, float ch_l, float ch_r);

//-----------------------------------------------------------------------------

#endif				// AUDIO_H

//-----------------------------------------------------------------------------
