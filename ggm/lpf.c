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
	f->kq = 2.f - 2.f * resonance;
}

void svf_init(struct svf *f) {
	// do nothing
}

//-----------------------------------------------------------------------------
// State Variable Filter
// https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

void svf2_gen(struct svf2 *f, float *out, const float *in, size_t n) {
	float ic1eq = f->ic1eq;
	float ic2eq = f->ic2eq;
	float a1 = f->a1;
	float a2 = f->a2;
	float a3 = f->a3;
	for (size_t i = 0; i < n; i++) {
		float v0, v1, v2, v3;
		v0 = in[i];
		v3 = v0 - ic2eq;
		v1 = (a1 * ic1eq) + (a2 * v3);
		v2 = ic2eq + (a2 * ic1eq) + (a3 * v3);
		ic1eq = (2.f * v1) - ic1eq;
		ic2eq = (2.f * v2) - ic2eq;
		out[i] = v2;	// low
		// low = v2;
		// band = v1;
		// high = v0 - (f->k * v1) - v2;
		// notch = v0 - (f->k * v1);
		// peak = v0 - (f->k * v1) - (2.f * v2);
		// all = v0 - (2.f * f->k * v1);
	}
	f->ic1eq = ic1eq;
	f->ic2eq = ic2eq;
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
