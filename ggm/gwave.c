//-----------------------------------------------------------------------------
/*

Goom Waves

The Goom synth has an oscillator whose wave shape is as follows:

1) A rising cos -pi to 0 curve (s0)
2) A flat piece at the top (f0)
3) A falling cos 0 to pi curve (s1)
4) A flat piece at the bottom (f1)

Shape is controller by two parameters:

duty = split the total period between s0,f0 and s1,f1
slope = split each slope/flat portion beween slope and flat.

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

void gwave_gen(struct dds *osc, float *out, size_t n) {
}

//-----------------------------------------------------------------------------

// Initialise a Goom wave.
// duty = duty cycle [0..1]
// slope = slope [0..1]
void gwave_init(struct gwave *osc, float duty, float slope) {

	uint32_t sf0, sf1, smin;

	memset(osc, 0, sizeof(struct gwave));

	duty = clamp(duty, 0.f, 1.f);
	slope = clamp(slope, 0.f, 1.f);

	// TODO
	smin = 40;

	// waveform has four periods: slope0, flat0, slope1, flat1
	// period for slope0, flat0 (we don't go to 0)
	sf0 = (1U << 31) - (uint32_t) ((float)(0x7c000000) * duty);
	// period for slope1, flat1
	sf1 = (1ULL << 32) - sf0;
	// slope0, flat0
	osc->s0 = smin + (uint32_t) ((float)(sf0 - smin) * slope);
	osc->f0 = sf0 - osc->s0;
	// slope1, flat1
	osc->s1 = smin + (uint32_t) ((float)(sf1 - smin) * slope);
	osc->f1 = sf1 - osc->s1;

	//DBG("s0 %08x f0 %08x s1 %08x f1 %08x sum %x\r\n", s0, f0, s1, f1, s0 + f0 + s1 + f1);

}

//-----------------------------------------------------------------------------
