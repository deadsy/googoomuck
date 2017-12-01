//-----------------------------------------------------------------------------
/*

Low Pass Filter

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

void lpf_gen(struct lpf *f, float *out, float *in, float *x, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] = in[i];
	}
}

//-----------------------------------------------------------------------------

void lpf_init(struct lpf *f, float cutoff, float resonance) {
	f->cutoff = cutoff;
	f->resonance = resonance;
}

//-----------------------------------------------------------------------------