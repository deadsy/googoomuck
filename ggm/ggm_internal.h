//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer Internals

*/
//-----------------------------------------------------------------------------

#ifndef GGM_INTERNAL_H
#define GGM_INTERNAL_H

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "audio.h"

//-----------------------------------------------------------------------------

#define AUDIO_FS ((float)AUDIO_SAMPLE_RATE)
#define AUDIO_TS (1.0f/AUDIO_FS)

#define PI (3.14159265358979f)
#define TAU (2.0f * PI)
#define INV_TAU (1.0f/TAU)

//-----------------------------------------------------------------------------
// lut oscillators

struct lut_osc {
	const uint32_t *table;	// lookup table
	size_t n;		// number of entries in LUT
	float amp;		// amplitude
	float freq;		// base frequency
	float xrange;		// x-range for LUT
	float fscale;		// frequency to x scaling (xrange * 1/fs)
	float x;		// current x-value
	float xstep;		// current x-step
};

// modulate the frequency of the oscillator
static inline void lut_mod_freq(struct lut_osc *osc, float f) {
	osc->xstep = (osc->freq + f) * osc->fscale;
}

float lut_sample(struct lut_osc *lut);
void lut_mod_freq(struct lut_osc *osc, float f);

//-----------------------------------------------------------------------------
// midi

static inline void major_chord(uint8_t * notes, uint8_t root) {
	notes[0] = root;
	notes[1] = root + 4;
	notes[2] = root + 7;
}

static inline void minor_chord(uint8_t * notes, uint8_t root) {
	notes[0] = root;
	notes[1] = root + 3;
	notes[2] = root + 7;
}

float midi_to_frequency(uint8_t note);

//-----------------------------------------------------------------------------
// oscillators

void osc_sin(struct lut_osc *osc, float amp, float freq, float phase);

//-----------------------------------------------------------------------------

#endif				// GGM_INTERNAL_H

//-----------------------------------------------------------------------------
