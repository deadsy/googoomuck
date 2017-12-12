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
	float kf = f->kf;
	float kq = f->kq;

	for (size_t i = 0; i < n; i++) {
		lp += kf * bp;
		float hp = in[i] - lp - (kq * bp);
		bp += kf * hp;
		out[i] = lp;
	}

	// update the state variables
	f->lp = lp;
	f->bp = bp;
}

// set the cutoff frequency
void svf_ctrl_cutoff(struct svf *f, float cutoff) {
	cutoff = clampf(cutoff, 0.f, 0.5f * AUDIO_FS);
	f->kf = 2.f * sin_eval(PI * cutoff / AUDIO_FS);
}

// set the resonance (0..1)
void svf_ctrl_resonance(struct svf *f, float resonance) {
	resonance = clampf(resonance, 0.f, 1.f);
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
	float a1 = 1.f / (1.f + (f->g * (f->g + f->k)));
	float a2 = f->g * a1;
	float a3 = f->g * a2;

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

	// update the state variables
	f->ic1eq = ic1eq;
	f->ic2eq = ic2eq;
}

// set the cutoff frequency
void svf2_ctrl_cutoff(struct svf2 *f, float cutoff) {
	cutoff = clampf(cutoff, 0.f, 0.5f * AUDIO_FS);
	f->g = tan_eval(PI * cutoff / AUDIO_FS);
}

// set the resonance (0..1)
void svf2_ctrl_resonance(struct svf2 *f, float resonance) {
	resonance = clampf(resonance, 0.f, 1.f);
	f->k = 2.f - 2.f * resonance;
}

void svf2_init(struct svf2 *f) {
	// do nothing
}

//-----------------------------------------------------------------------------
