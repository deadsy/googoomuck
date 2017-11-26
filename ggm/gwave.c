//-----------------------------------------------------------------------------
/*

Goom Waves

A Goom Wave is a wave shape with the following segments:

1) s0: A falling (1 to -1) sine curve
2) f0: A flat piece at the bottom
3) s1: A rising (-1 to 1) sine curve
4) f1: A flat piece at the top

Shape is controller by two parameters:
duty = split the total period between s0,f0 and s1,f1
slope = split s0f0 and s1f1 beween slope and flat.

The idea for goom waves comes from: https://www.quinapalus.com/goom.html

*/
//-----------------------------------------------------------------------------

#include <math.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// N bits to represent the phase
#define PHASE_BITS (16U)
#define PHASE_MAX (1U << PHASE_BITS)

// This divisor maps the phase value onto the index used for the lookup table.
#define SLOPE_DIV (16U)

// Limit how close the duty cyle can get to 0/100%.
#define TP_MIN (PHASE_MAX >> 6)

// Limit how fast the slope can rise.
#define S_MIN 0.1f

// frequency to x scaling (xrange/fs)
#define GWAVE_FSCALE ((float)PHASE_MAX / AUDIO_FS)

//-----------------------------------------------------------------------------

// generated by ./scripts/lut.py
#define COS_BITS (6U)
#define COS_SIZE (1U << 6)
static const uint32_t COS_data[COS_SIZE << 1] = {
	0x3f800000U, 0xba9de1c8U, 0x3f7fb10fU, 0xbb6ca1fdU, 0x3f7ec46dU, 0xbbc4e093U, 0x3f7d3aacU, 0xbc097b5eU,
	0x3f7b14beU, 0xbc3031a9U, 0x3f7853f8U, 0xbc567b4aU, 0x3f74fa0bU, 0xbc7c40a4U, 0x3f710908U, 0xbc90b536U,
	0x3f6c835eU, 0xbca2f0dbU, 0x3f676bd8U, 0xbcb4c803U, 0x3f61c598U, 0xbcc62facU, 0x3f5b941aU, 0xbcd71d1cU,
	0x3f54db31U, 0xbce785e1U, 0x3f4d9f02U, 0xbcf75fddU, 0x3f45e403U, 0xbd0350a4U, 0x3f3daef9U, 0xbd0aa05eU,
	0x3f3504f3U, 0xbd119a99U, 0x3f2beb4aU, 0xbd183b08U, 0x3f226799U, 0xbd1e7d94U, 0x3f187fc0U, 0xbd245e62U,
	0x3f0e39daU, 0xbd29d9d0U, 0x3f039c3dU, 0xbd2eec7fU, 0x3ef15aeaU, 0xbd33934cU, 0x3edae880U, 0xbd37cb59U,
	0x3ec3ef15U, 0xbd3b920cU, 0x3eac7cd4U, 0xbd3ee512U, 0x3e94a031U, 0xbd41c25cU, 0x3e78cfccU, 0xbd442828U,
	0x3e47c5c2U, 0xbd4614faU, 0x3e164083U, 0xbd4787a2U, 0x3dc8bd36U, 0xbd487f3cU, 0x3d48fb30U, 0xbd48fb30U,
	0x248d3132U, 0xbd48fb30U, 0xbd48fb30U, 0xbd487f3cU, 0xbdc8bd36U, 0xbd4787a2U, 0xbe164083U, 0xbd4614faU,
	0xbe47c5c2U, 0xbd442828U, 0xbe78cfccU, 0xbd41c25cU, 0xbe94a031U, 0xbd3ee512U, 0xbeac7cd4U, 0xbd3b920cU,
	0xbec3ef15U, 0xbd37cb59U, 0xbedae880U, 0xbd33934cU, 0xbef15aeaU, 0xbd2eec7fU, 0xbf039c3dU, 0xbd29d9d0U,
	0xbf0e39daU, 0xbd245e62U, 0xbf187fc0U, 0xbd1e7d94U, 0xbf226799U, 0xbd183b08U, 0xbf2beb4aU, 0xbd119a99U,
	0xbf3504f3U, 0xbd0aa05eU, 0xbf3daef9U, 0xbd0350a4U, 0xbf45e403U, 0xbcf75fddU, 0xbf4d9f02U, 0xbce785e1U,
	0xbf54db31U, 0xbcd71d1cU, 0xbf5b941aU, 0xbcc62facU, 0xbf61c598U, 0xbcb4c803U, 0xbf676bd8U, 0xbca2f0dbU,
	0xbf6c835eU, 0xbc90b536U, 0xbf710908U, 0xbc7c40a4U, 0xbf74fa0bU, 0xbc567b4aU, 0xbf7853f8U, 0xbc3031a9U,
	0xbf7b14beU, 0xbc097b5eU, 0xbf7d3aacU, 0xbbc4e093U, 0xbf7ec46dU, 0xbb6ca1fdU, 0xbf7fb10fU, 0xba9de1c8U,
};

//-----------------------------------------------------------------------------

void gwave_gen(struct gwave *osc, float *out, float *fm, size_t n) {
	for (size_t i = 0; i < n; i++) {
		int inv = 0;
		uint32_t sx;

		// what portion of the goom wave are we in?
		if (osc->x < osc->tp) {
			// we are in the s0/f0 portion
			sx = osc->x * osc->k0;
		} else {
			// we are in the s1/f1 portion
			sx = (osc->x - osc->tp) * osc->k1;
			inv = 1;
		}

		uint32_t idx = __USAT(sx >> SLOPE_DIV, COS_BITS) << 1;
		float frac = (float)(sx & ((1 << SLOPE_DIV) - 1)) / (float)(1 << SLOPE_DIV);
		float y = *(float *)&COS_data[idx];
		float dy = *(float *)&COS_data[idx + 1];
		y += (frac * dy);
		out[i] = (inv) ? -y : y;

		// step the phase
		if (fm) {
			osc->x += (uint32_t) ((osc->freq + fm[i]) * GWAVE_FSCALE);
		} else {
			osc->x += osc->xstep;
		}
		osc->x &= (PHASE_MAX - 1);
	}
}

//-----------------------------------------------------------------------------

// Control the shape of the Goom wave.
// duty = duty cycle 0..1
// slope = slope 0..1
void gwave_shape(struct gwave *osc, float duty, float slope) {
	duty = clamp(duty, 0.f, 1.f);
	slope = clamp(slope, 0.f, 1.f);
	// This is where we transition from s0f0 to s1f1.
	osc->tp = (PHASE_MAX >> 1) - (uint32_t) ((float)((PHASE_MAX >> 1) - TP_MIN) * duty);
	// Work out the portion of s0f0/s1f1 that is sloped.
	float s = S_MIN + (1.f - S_MIN) * slope;
	// scaling constant for s0, map the slope to the LUT.
	osc->k0 = (COS_SIZE << SLOPE_DIV) / (uint32_t) ((float)osc->tp * s);
	// scaling constant for s1, map the slope to the LUT.
	osc->k1 = (COS_SIZE << SLOPE_DIV) / (uint32_t) ((float)(PHASE_MAX - osc->tp) * s);
}

//-----------------------------------------------------------------------------

// Initialise a Goom wave.
void gwave_init(struct gwave *osc, float freq, float phase) {
	// frequency
	osc->freq = freq;
	osc->xstep = (uint32_t) (osc->freq * GWAVE_FSCALE);
	// phase
	osc->phase = fmodf(phase, TAU);
}

//-----------------------------------------------------------------------------
