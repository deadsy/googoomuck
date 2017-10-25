//-----------------------------------------------------------------------------
/*

Cirrus Logic CS43L22 Stereo DAC

*/
//-----------------------------------------------------------------------------

#ifndef CS43L22_H
#define CS43L22_H

//-----------------------------------------------------------------------------

#include <inttypes.h>

//-----------------------------------------------------------------------------

enum {
	DAC_OUTPUT_OFF,		// must be 0
	DAC_OUTPUT_SPEAKER,
	DAC_OUTPUT_HEADPHONE,
	DAC_OUTPUT_BOTH,
	DAC_OUTPUT_AUTO,
	DAC_OUTPUT_MAX,		// must be last
};

//-----------------------------------------------------------------------------

struct cs4x_cfg {
	struct i2c_drv *i2c;	// i2c bus
	uint8_t adr;		// device i2c bus address
	int rst;		// gpio for reset pin
	int out;		// output device
};

struct cs4x_drv {
	struct cs4x_cfg cfg;
};

//-----------------------------------------------------------------------------

int cs4x_init(struct cs4x_drv *dac, struct cs4x_cfg *cfg);

int cs4x_start(struct cs4x_drv *dac);
int cs4x_stop(struct cs4x_drv *dac);
int cs4x_pause(struct cs4x_drv *dac);
int cs4x_resume(struct cs4x_drv *dac);

int cs4x_output(struct cs4x_drv *dac, unsigned int out);
int cs4x_master_volume(struct cs4x_drv *dac, uint8_t vol);
int cs4x_headphone_volume(struct cs4x_drv *dac, uint8_t vol);
int cs4x_speaker_volume(struct cs4x_drv *dac, uint8_t vol);
int cs4x_pcm_volume(struct cs4x_drv *dac, uint8_t vol);

//-----------------------------------------------------------------------------

#endif				// CS43L22_H

//-----------------------------------------------------------------------------
