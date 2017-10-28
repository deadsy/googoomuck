//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer

*/
//-----------------------------------------------------------------------------

#ifndef GGM_H
#define GGM_H

//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stddef.h>

#include "audio.h"

//-----------------------------------------------------------------------------

#define AUDIO_TS (1.0f/AUDIO_FS)

#define PI (3.1415927f)
#define TAU (2.0f * PI)
#define INV_TAU (1.0f/TAU)

//-----------------------------------------------------------------------------
// LUT oscillators

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

// oscillators
void osc_sin(struct lut_osc *osc, float amp, float freq, float phase);

//-----------------------------------------------------------------------------
// ADSR envelope

struct adsr {
	float s;		// sustain level
	float ka;		// attack constant
	float kd;		// decay constant
	float kr;		// release constant
	float d_trigger;	// attack->decay trigger level
	float s_trigger;	// decay->sustain trigger level
	float i_trigger;	// release->idle trigger level
	int state;		// envelope state
	float val;		// output value
};

float adsr_sample(struct adsr *e);

// envelopes
void adsr_init(struct adsr *e, float a, float d, float s, float r);
void ad_init(struct adsr *e, float a, float d);

// actions
void adsr_attack(struct adsr *e);
void adsr_release(struct adsr *e);
void adsr_idle(struct adsr *e);

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

#define MIDI_MSG(t, c, x, y) \
  (((t) & 15) << 20 /*type*/) | \
  (((c) & 15) << 16 /*channel*/) | \
  (((x) & 127) << 8 /*x-parameter*/) | \
  ((y) & 127 /*y-parameter*/)

// channel, note, velocity
#define MIDI_NOTE_ON(c, n, v) MIDI_MSG(9 /*note on*/, c, n, v)
#define MIDI_NOTE_OFF(c, n, v) MIDI_MSG(8 /*note off*/, c, n, v)

//-----------------------------------------------------------------------------
// events

// event type in the upper 8 bits
#define EVENT_TYPE(x) ((x) & 0xff000000U)
#define EVENT_TYPE_KEY_DN (1U << 24)
#define EVENT_TYPE_KEY_UP (2U << 24)
#define EVENT_TYPE_MIDI (3U << 24)

// key number in the lower 8 bits
#define EVENT_KEY(x) ((x) & 0xffU)
// midi message in the lower 3 bytes
#define EVENT_MIDI(x) ((x) & 0xffffffU)

struct event {
	uint32_t type;		// the event type
	void *ptr;		// pointer to event data (or the data itself)
};

int event_init(void);
int event_rd(struct event *event);
int event_wr(uint32_t type, void *ptr);

//-----------------------------------------------------------------------------

struct ggm_state {
	struct audio_drv *audio;
	struct lut_osc sin;
	struct adsr adsr;
};

//-----------------------------------------------------------------------------

int ggm_init(struct ggm_state *s, struct audio_drv *audio);
int ggm_run(struct ggm_state *s);

//-----------------------------------------------------------------------------

#endif				// GGM_H

//-----------------------------------------------------------------------------
