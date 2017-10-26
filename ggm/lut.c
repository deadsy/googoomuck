//-----------------------------------------------------------------------------
/*

Lookup Table Based Oscillators

*/
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include "ggm_internal.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

#define COS_TABLE_SIZE 64U
static const uint32_t cos_table[COS_TABLE_SIZE] = {
	0x3f800000U, 0x3f7ec46dU, 0x3f7b14beU, 0x3f74fa0bU,
	0x3f6c835eU, 0x3f61c598U, 0x3f54db31U, 0x3f45e403U,
	0x3f3504f3U, 0x3f226799U, 0x3f0e39daU, 0x3ef15aeaU,
	0x3ec3ef15U, 0x3e94a031U, 0x3e47c5c2U, 0x3dc8bd36U,
	0x248d3132U, 0xbdc8bd36U, 0xbe47c5c2U, 0xbe94a031U,
	0xbec3ef15U, 0xbef15aeaU, 0xbf0e39daU, 0xbf226799U,
	0xbf3504f3U, 0xbf45e403U, 0xbf54db31U, 0xbf61c598U,
	0xbf6c835eU, 0xbf74fa0bU, 0xbf7b14beU, 0xbf7ec46dU,
	0xbf800000U, 0xbf7ec46dU, 0xbf7b14beU, 0xbf74fa0bU,
	0xbf6c835eU, 0xbf61c598U, 0xbf54db31U, 0xbf45e403U,
	0xbf3504f3U, 0xbf226799U, 0xbf0e39daU, 0xbef15aeaU,
	0xbec3ef15U, 0xbe94a031U, 0xbe47c5c2U, 0xbdc8bd36U,
	0xa553c9caU, 0x3dc8bd36U, 0x3e47c5c2U, 0x3e94a031U,
	0x3ec3ef15U, 0x3ef15aeaU, 0x3f0e39daU, 0x3f226799U,
	0x3f3504f3U, 0x3f45e403U, 0x3f54db31U, 0x3f61c598U,
	0x3f6c835eU, 0x3f74fa0bU, 0x3f7b14beU, 0x3f7ec46dU,
};

//-----------------------------------------------------------------------------

// return a sample from a lookup table based oscillator
float lut_sample(struct lut_osc *osc) {
	size_t x0 = (size_t) osc->x;
	float y0 = *(float *)&osc->table[x0];
	float y, y1;
	if (x0 == osc->n - 1) {
		y1 = *(float *)&osc->table[0];
	} else {
		y1 = *(float *)&osc->table[x0 + 1];
	}
	// interpolate
	y = y0 + ((osc->x - (float)x0) * (y1 - y0));
	// step the x position
	osc->x += osc->xstep;
	if (osc->x >= osc->xrange) {
		osc->x -= osc->xrange;
	}
	return osc->amp * y;
}

//-----------------------------------------------------------------------------

// initialise a sine wave oscillator
void osc_sin(struct lut_osc *osc, float amp, float freq, float phase) {
	memset(osc, 0, sizeof(struct lut_osc));
	// setup the table
	osc->table = cos_table;
	osc->n = COS_TABLE_SIZE;
	osc->xrange = (float)COS_TABLE_SIZE;
	osc->fscale = osc->xrange * AUDIO_TS;
	// amplitude
	osc->amp = amp;
	// phase
	osc->x = osc->xrange * fmodf(phase, TAU);
	// frequency
	osc->freq = freq;
	lut_mod_freq(osc, 0.0f);
}

//-----------------------------------------------------------------------------
