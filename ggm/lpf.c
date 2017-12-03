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
// See: Hal Chamberlin's "Musical Applications of Microprocessors".

void svf_gen(struct svf *f, float *out, float *in, float *x, size_t n) {
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

void svf_update_cutoff(struct svf *f, float cutoff) {
	f->kf = 2.f * sin_eval(PI * cutoff / AUDIO_FS);
}

void svf_update_resonance(struct svf *f, float resonance) {
	f->kq = 1.f / clampf_lo(resonance, 0.5);
}

void svf_init(struct svf *f, float cutoff, float resonance) {
	svf_update_cutoff(f, cutoff);
	svf_update_resonance(f, resonance);
}

//-----------------------------------------------------------------------------
