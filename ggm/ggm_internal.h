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

//-----------------------------------------------------------------------------
// lut oscillators

struct lut_osc {
	const uint32_t *table;
	uint32_t n;
	float xstep;
	float xrange;
	float x;
};

float lut_sample(struct lut_osc *lut);
void lut_set_frequency(struct lut_osc *osc, float f, uint32_t rate);

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

void osc_sin(struct lut_osc *osc, float f, uint32_t rate);

//-----------------------------------------------------------------------------

#endif				// GGM_INTERNAL_H

//-----------------------------------------------------------------------------
