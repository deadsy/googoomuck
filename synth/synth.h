//-----------------------------------------------------------------------------
/*

Synthesizer

*/
//-----------------------------------------------------------------------------

#ifndef SYNTH_H
#define SYNTH_H

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

//-----------------------------------------------------------------------------

struct osc_lut {
	const uint32_t *table;
	uint32_t n;
	float xstep;
	float xrange;
	float x;
};

//-----------------------------------------------------------------------------

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

int osc_sin_init(struct osc_lut *osc, float f, uint32_t rate);
float lut_sample(struct osc_lut *osc);

//-----------------------------------------------------------------------------

#endif				// SYNTH_H

//-----------------------------------------------------------------------------
