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

static inline void block_mul(float *out, float *k, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		out[i] *= k[i];
	}
}

//-----------------------------------------------------------------------------
// DDS Oscillators

struct dds {
	const float *table;	// lookup table
	uint32_t table_mask;	// mask for the table bits
	uint32_t frac_bits;	// number of fraction bits
	uint32_t frac_mask;	// mask for fraction bits
	uint32_t x;		// current x-value
	uint32_t xstep;		// current x-step
	float frac_scale;	// scaling for the fractional portion
	float phase;		// base phase
	float freq;		// base frequency
	float amp;		// amplitude
};

// oscillators
void dds_sin_init(struct dds *osc, float amp, float freq, float phase);

// generators
void dds_gen(struct dds *osc, float *out, size_t n);
void dds_gen_am(struct dds *osc, float *out, float *am, size_t n);
void dds_gen_fm(struct dds *osc, float *out, float *fm, size_t n);
void dds_gen_fm_am(struct dds *osc, float *out, float *fm, float *am, size_t n);

//-----------------------------------------------------------------------------
// Goom Waves

struct gwave {
	uint32_t x;		// phase position
	uint32_t xstep;		// phase step per sample
	uint32_t tp;		// s0f0 to s1f1 transition point
	uint32_t k0;		// scaling factor for slope 0
	uint32_t k1;		// scaling factor for slope 1
	float phase;		// base phase
	float freq;		// base frequency
	float amp;		// amplitude
};

void gwave_shape(struct gwave *osc, float duty, float slope);
void gwave_init(struct gwave *osc, float duty, float slope, float amp, float freq, float phase);
void gwave_gen(struct gwave *osc, float *out, size_t n);
void gwave_gen_am(struct gwave *osc, float *out, float *am, size_t n);

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

// generators
void adsr_gen(struct adsr *e, float *out, size_t n);

// envelopes
void adsr_init(struct adsr *e, float a, float d, float s, float r);
void ad_init(struct adsr *e, float a, float d);

// actions
void adsr_attack(struct adsr *e);
void adsr_release(struct adsr *e);
void adsr_idle(struct adsr *e);

// state
int adsr_is_active(struct adsr *e);

//-----------------------------------------------------------------------------
// midi

float midi_to_frequency(uint8_t note);

struct midi_drv {
	struct usart_drv serial_drv;
	int state;
	uint8_t channel;
	uint8_t arg0;
	uint8_t arg1;
	void (*func) (struct midi_drv * midi);
};

int midi_init(struct midi_drv *midi, uint32_t base);
void midi_process(struct midi_drv *midi);

//-----------------------------------------------------------------------------
// events

// event type in the upper 8 bits
#define EVENT_TYPE(x) ((x) & 0xff000000U)
#define EVENT_TYPE_KEY_DN (1U << 24)
#define EVENT_TYPE_KEY_UP (2U << 24)
#define EVENT_TYPE_MIDI (3U << 24)
#define EVENT_TYPE_AUDIO (4U << 24)

// key number in the lower 8 bits
#define EVENT_KEY(x) ((x) & 0xffU)
// midi message in the lower 3 bytes
#define EVENT_MIDI(x) ((x) & 0xffffffU)
// audio block size in the lower 16 bits
#define EVENT_BLOCK_SIZE(x) ((x) & 0xffffU)

struct event {
	uint32_t type;		// the event type
	void *ptr;		// pointer to event data (or data itself)
};

int event_init(void);
int event_rd(struct event *event);
int event_wr(uint32_t type, void *ptr);

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

//-----------------------------------------------------------------------------

struct ggm_state {
	struct audio_drv *audio;
	struct midi_drv *midi;
	struct dds lfo;
	struct dds sin;
	struct adsr adsr;
	struct gwave gw;
};

int ggm_init(struct ggm_state *s, struct audio_drv *audio, struct midi_drv *midi);
int ggm_run(struct ggm_state *s);

//-----------------------------------------------------------------------------

#endif				// GGM_H

//-----------------------------------------------------------------------------
