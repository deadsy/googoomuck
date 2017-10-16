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
	DAC_OUTPUT_OFF,
	DAC_OUTPUT_SPEAKER,
	DAC_OUTPUT_HEADPHONE,
	DAC_OUTPUT_BOTH,
	DAC_OUTPUT_AUTO,
	DAC_OUTPUT_MAX,		// must be last
};

//-----------------------------------------------------------------------------

struct cs4x_dac {
	struct i2c_bus *i2c;	// i2c bus
	uint8_t adr;		// device i2c bus address
	int rst;		// gpio for reset pin
};

//-----------------------------------------------------------------------------

int cs4x_init(struct cs4x_dac *dac, struct i2c_bus *i2c, uint8_t adr, int rst);
int cs4x_output(struct cs4x_dac *dac, unsigned int out);

//-----------------------------------------------------------------------------

#endif				// CS43L22_H

//-----------------------------------------------------------------------------
