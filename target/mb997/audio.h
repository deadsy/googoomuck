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
// This is used to lookup the i2s clock configuration in a constant table.
// This is turned into AUDIO_FS (frequency) and AUDIO_TS (period) floats
// for various scaling operations in the synth routines.

#define AUDIO_SAMPLE_RATE 35156U	// Hz

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
