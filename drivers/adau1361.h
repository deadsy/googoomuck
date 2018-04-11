//-----------------------------------------------------------------------------
/*

Analog Device ADAU1361 Stereo CODEC

*/
//-----------------------------------------------------------------------------

#ifndef ADAU1361_H
#define ADAU1361_H

//-----------------------------------------------------------------------------

#include <inttypes.h>

//-----------------------------------------------------------------------------

struct adau1361_cfg {
	struct i2c_drv *i2c;	// i2c bus
	uint8_t adr;		// device i2c bus address
};

struct adau1361_drv {
	struct adau1361_cfg cfg;
};

//-----------------------------------------------------------------------------

int adau1361_init(struct adau1361_drv *codec, struct adau1361_cfg *cfg);

int adau1361_start(struct adau1361_drv *codec);
//int cs4x_stop(struct cs4x_drv *dac);
//int cs4x_pause(struct cs4x_drv *dac);
//int cs4x_resume(struct cs4x_drv *dac);

//int cs4x_output(struct cs4x_drv *dac, unsigned int out);
int adau1361_master_volume(struct adau1361_drv *codec, uint8_t vol);
//int cs4x_headphone_volume(struct cs4x_drv *dac, uint8_t vol);
//int cs4x_speaker_volume(struct cs4x_drv *dac, uint8_t vol);
//int cs4x_pcm_volume(struct cs4x_drv *dac, uint8_t vol);

//-----------------------------------------------------------------------------

#endif				// ADAU1361_H

//-----------------------------------------------------------------------------
