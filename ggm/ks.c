//-----------------------------------------------------------------------------
/*

Karplus Strong Plucked String Modelling

KS generally has a delay line buffer size that determines the fundamental frequency
of the sound. That has some practical problems. The delay line buffer is too
large for low frequencies and it makes it hard to provide fine resolution
control over the frequency. This implementation uses a fixed buffer size and
steps through it with a 32 bit phase value. The step size determines the
frequency of the sound. When the step position falls between samples we do
linear interpolation to get the output value. When we move beyond a sample
we do the low pass filtering on it (in this case simple averaging).

*/
//-----------------------------------------------------------------------------

#include "ggm.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// frequency to x scaling (xrange/fs)
#define KS_FSCALE ((float)(1ULL << 32) / AUDIO_FS)

#define KS_DELAY_MASK (KS_DELAY_SIZE - 1)
#define KS_FRAC_BITS (32U - KS_DELAY_BITS)
#define KS_FRAC_MASK ((1U << KS_FRAC_BITS) - 1)
#define KS_FRAC_SCALE (float)(1.f / (float)(1ULL << KS_FRAC_BITS))

//-----------------------------------------------------------------------------

void ks_gen(struct ks *osc, float *out, size_t n) {
	for (size_t i = 0; i < n; i++) {
		unsigned int x0 = osc->x >> KS_FRAC_BITS;
		unsigned int x1 = (x0 + 1) & KS_DELAY_MASK;
		float y0 = osc->delay[x0];
		float y1 = osc->delay[x1];
		// interpolate
		out[i] = y0 + (y1 - y0) * KS_FRAC_SCALE * (float)(osc->x & KS_FRAC_MASK);
		// step the x position
		osc->x += osc->xstep;
		// filter - once we have moved beyond the delay line index we
		// will average it's amplitude with the next value.
		if (x0 != (osc->x >> KS_FRAC_BITS)) {
			osc->delay[x0] = osc->k * (y0 + y1);
		}
	}
}

//-----------------------------------------------------------------------------

void ks_pluck(struct ks *osc) {
	// Initialise the delay buffer with random samples between -1 and 1.
	// The values should sum to zero so that multiple rounds of filtering
	// will make all values fall to zero.
	float sum = 0.f;
	for (unsigned int i = 0; i < KS_DELAY_SIZE - 1; i++) {
		float val = rand_float();
		float x = sum + val;
		if (x > 1.f || x < -1.f) {
			val = -val;
		}
		sum += val;
		osc->delay[i] = val;
	}
	osc->delay[KS_DELAY_SIZE - 1] = -sum;
}

//-----------------------------------------------------------------------------

void ks_ctrl_attenuate(struct ks *osc, float attenuate) {
	osc->k = 0.5f * attenuate;
}

void ks_ctrl_frequency(struct ks *osc, float freq) {
	osc->freq = freq;
	osc->xstep = (uint32_t) (osc->freq * KS_FSCALE);
}

void ks_init(struct ks *osc) {
	// do nothing
}

//-----------------------------------------------------------------------------
