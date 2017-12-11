//-----------------------------------------------------------------------------
/*

Low Pass Filters

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// State Variable Filter
// See: Hal Chamberlin's "Musical Applications of Microprocessors" pp.489-492.

void svf_gen(struct svf *f, float *out, const float *in, size_t n) {
	float lp = f->lp;
	float bp = f->bp;
	for (size_t i = 0; i < n; i++) {
		lp += f->kf * bp;
		float hp = in[i] - lp - (f->kq * bp);
		bp += f->kf * hp;
		out[i] = lp;
	}
	// update the state variables
	f->lp = lp;
	f->bp = bp;
}

void svf_ctrl_cutoff(struct svf *f, float cutoff) {
	f->kf = 2.f * sin_eval(PI * cutoff / AUDIO_FS);
}

void svf_ctrl_resonance(struct svf *f, float resonance) {
	f->kq = 1.f / clampf_lo(resonance, 0.5);
}

void svf_init(struct svf *f) {
	// do nothing
}

//-----------------------------------------------------------------------------
// State Variable Filter
// https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

void svf2_gen(struct svf2 *f, float *out, const float *in, size_t n) {
	float v0, v1, v2, v3;
	for (size_t i = 0; i < n; i++) {
		v0 = in[i];
		v3 = v0 - f->ic2eq;
		v1 = (f->a1 * f->ic1eq) + (f->a2 * v3);
		v2 = f->ic2eq + (f->a2 * f->ic1eq) + (f->a3 * v3);
		f->ic1eq = (2.f * v1) - f->ic1eq;
		f->ic2eq = (2.f * v2) - f->ic2eq;
		out[i] = v2;	// low
		// low = v2;
		// band = v1;
		// high = v0 - f->k * v1 - v2;
		// notch = v0 - f->k * v1;
		// peak = v0 - f->k * v1 - 2.f * v2;
		// all = v0 - 2.f * f->k * v1;
	}
}

static void svf_ctrl_update(struct svf2 *f) {
	f->a1 = 1.f / (1.f + (f->g * (f->g + f->k)));
	f->a2 = f->g * f->a1;
	f->a3 = f->g * f->a2;
}

void svf2_ctrl_cutoff(struct svf2 *f, float cutoff) {
	f->g = tan_eval(PI * cutoff / AUDIO_FS);
	svf_ctrl_update(f);
}

void svf2_ctrl_resonance(struct svf2 *f, float resonance) {
	f->k = 2.f - 2.f * resonance;
	svf_ctrl_update(f);
}

void svf2_init(struct svf2 *f) {
	// do nothing
}

//-----------------------------------------------------------------------------
