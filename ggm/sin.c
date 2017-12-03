//-----------------------------------------------------------------------------
/*

Sine Wave Oscillators

*/
//-----------------------------------------------------------------------------

#include <math.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// generated by ./scripts/lut.py
#define COS_LUT_BITS (7U)
#define COS_LUT_SIZE (1U << COS_LUT_BITS)
static const int32_t COS_LUT_data[COS_LUT_SIZE << 1] = {
	1073741824, -1293369, 1072448454, -3876991, 1068571463, -6451273, 1062120190, -9010014,
	1053110175, -11547048, 1041563127, -14056265, 1027506861, -16531619, 1010975241, -18967147,
	992008094, -21356981, 970651112, -23695365, 946955747, -25976664, 920979082, -28195384,
	892783698, -30346178, 862437519, -32423865, 830013654, -34423441, 795590212, -36340087,
	759250124, -38169187, 721080937, -39906335, 681174602, -41547344, 639627257, -43088262,
	596538995, -44525377, 552013618, -45855226, 506158392, -47074606, 459083785, -48180579,
	410903206, -49170481, 361732725, -50041926, 311690798, -50792817, 260897981, -51421343,
	209476638, -51925990, 157550647, -52305544, 105245103, -52559088, 52686014, -52686014,
	0, -52686014, -52686014, -52559088, -105245103, -52305544, -157550647, -51925990,
	-209476638, -51421343, -260897981, -50792817, -311690798, -50041926, -361732725, -49170481,
	-410903206, -48180579, -459083785, -47074606, -506158392, -45855226, -552013618, -44525377,
	-596538995, -43088262, -639627257, -41547344, -681174602, -39906335, -721080937, -38169187,
	-759250124, -36340087, -795590212, -34423441, -830013654, -32423865, -862437519, -30346178,
	-892783698, -28195384, -920979082, -25976664, -946955747, -23695365, -970651112, -21356981,
	-992008094, -18967147, -1010975241, -16531619, -1027506861, -14056265, -1041563127, -11547048,
	-1053110175, -9010014, -1062120190, -6451273, -1068571463, -3876991, -1072448454, -1293369,
	-1073741824, 1293369, -1072448454, 3876991, -1068571463, 6451273, -1062120190, 9010014,
	-1053110175, 11547048, -1041563127, 14056265, -1027506861, 16531619, -1010975241, 18967147,
	-992008094, 21356981, -970651112, 23695365, -946955747, 25976664, -920979082, 28195384,
	-892783698, 30346178, -862437519, 32423865, -830013654, 34423441, -795590212, 36340087,
	-759250124, 38169187, -721080937, 39906335, -681174602, 41547344, -639627257, 43088262,
	-596538995, 44525377, -552013618, 45855226, -506158392, 47074606, -459083785, 48180579,
	-410903206, 49170481, -361732725, 50041926, -311690798, 50792817, -260897981, 51421343,
	-209476638, 51925990, -157550647, 52305544, -105245103, 52559088, -52686014, 52686014,
	0, 52686014, 52686014, 52559088, 105245103, 52305544, 157550647, 51925990,
	209476638, 51421343, 260897981, 50792817, 311690798, 50041926, 361732725, 49170481,
	410903206, 48180579, 459083785, 47074606, 506158392, 45855226, 552013618, 44525377,
	596538995, 43088262, 639627257, 41547344, 681174602, 39906335, 721080937, 38169187,
	759250124, 36340087, 795590212, 34423441, 830013654, 32423865, 862437519, 30346178,
	892783698, 28195384, 920979082, 25976664, 946955747, 23695365, 970651112, 21356981,
	992008094, 18967147, 1010975241, 16531619, 1027506861, 14056265, 1041563127, 11547048,
	1053110175, 9010014, 1062120190, 6451273, 1068571463, 3876991, 1072448454, 1293369,
};

#define FRAC_BITS (32U - COS_LUT_BITS)
#define FRAC_MASK ((1U << FRAC_BITS) - 1)
#define COS_SCALE (1.f/(float)(1U << 30))

// frequency to x scaling (xrange/fs)
#define FREQ_SCALE ((float)(1ULL << 32) / AUDIO_FS)

//-----------------------------------------------------------------------------

float cos_lookup(uint32_t x) {
	uint32_t idx = (x >> FRAC_BITS) << 1;
	int32_t frac = x & FRAC_MASK;
	int32_t y = COS_LUT_data[idx];
	int32_t dy = COS_LUT_data[idx + 1];
	y += ((int64_t) frac * (int64_t) dy) >> FRAC_BITS;
	return (float)y *COS_SCALE;
}

//-----------------------------------------------------------------------------
// LUT based equivalents to cosf/sinf

#define PHASE_SCALE ((float)(1ULL << 32)/TAU)

float cos_eval(float x) {
	uint32_t xi = (uint32_t) (fabs(x) * PHASE_SCALE);
	return cos_lookup(xi);
}

float sin_eval(float x) {
	uint32_t xi = (1U << 30) - (uint32_t) (fabs(x) * PHASE_SCALE);
	return cos_lookup(xi);
}

//-----------------------------------------------------------------------------

void sin_gen(struct sin *osc, float *out, float *fm, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] = cos_lookup(osc->x);
		// step the phase
		if (fm) {
			osc->x += (uint32_t) ((osc->freq + fm[i]) * FREQ_SCALE);
		} else {
			osc->x += osc->xstep;
		}
	}
}

void sin_init(struct sin *osc, float freq) {
	osc->freq = freq;
	osc->xstep = (uint32_t) (osc->freq * FREQ_SCALE);
}

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

// Limit how close the duty cyle can get to 0/100%.
#define TP_MIN 0.05f

// Limit how fast the slope can rise.
#define SLOPE_MIN 0.1f

#define FULL_CYCLE ((float)(1ULL << 32))
#define HALF_CYCLE (1 << 31)

void gwave_gen(struct gwave *osc, float *out, float *fm, size_t n) {
	for (size_t i = 0; i < n; i++) {
		uint64_t x;
		uint32_t ofs;

		// what portion of the goom wave are we in?
		if (osc->x < osc->tp) {
			// we are in the s0/f0 portion
			x = (uint64_t) osc->x * (uint64_t) osc->k0;
			ofs = 0;
		} else {
			// we are in the s1/f1 portion
			x = (uint64_t) (osc->x - osc->tp) * (uint64_t) osc->k1;
			ofs = HALF_CYCLE;
		}
		out[i] = cos_lookup(__USAT(x, 31) + ofs);

		// step the phase
		if (fm) {
			osc->x += (uint32_t) ((osc->freq + fm[i]) * FREQ_SCALE);
		} else {
			osc->x += osc->xstep;
		}
	}
}

// Control the shape of the Goom wave.
// duty = duty cycle 0..1
// slope = slope 0..1
void gwave_shape(struct gwave *osc, float duty, float slope) {
	duty = clampf(duty, 0.f, 1.f);
	slope = clampf(slope, 0.f, 1.f);
	// This is where we transition from s0f0 to s1f1.
	osc->tp = (uint32_t) (FULL_CYCLE * mapf(duty, TP_MIN, 1.f - TP_MIN));
	// Work out the portion of s0f0/s1f1 that is sloped.
	float s = mapf(slope, SLOPE_MIN, 1.0);

	// TODO fix

	// scaling constant for s0, map the slope to the LUT.
	osc->k0 = HALF_CYCLE / (uint32_t) ((float)osc->tp * s);
	// scaling constant for s1, map the slope to the LUT.
	osc->k1 = HALF_CYCLE / (uint32_t) ((FULL_CYCLE - (float)osc->tp) * s);

	DBG("tp %08x k0 %08x k1 %08x\r\n", osc->tp, osc->k0, osc->k1);

}

void gwave_init(struct gwave *osc, float freq) {
	osc->freq = freq;
	osc->xstep = (uint32_t) (osc->freq * FREQ_SCALE);
}

//-----------------------------------------------------------------------------
