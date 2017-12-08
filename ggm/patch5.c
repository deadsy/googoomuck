//-----------------------------------------------------------------------------
/*

Patch 5 - FM Synthesis

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct sin modulator;
	struct sin carrier;
	struct adsr aeg;
};

struct p_state {
	float bend;
	float modulator_level;
	float modulator_factor;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	float freq = midi_to_frequency((float)v->note + ps->bend);
	sin_ctrl_frequency(&vs->modulator, ps->modulator_factor * freq);
	sin_ctrl_frequency(&vs->carrier, freq);
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	DBG("p5 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	memset(vs, 0, sizeof(struct v_state));
	sin_init(&vs->modulator);
	sin_init(&vs->carrier);
	adsr_init(&vs->aeg, 0.05f, 0.2f, 0.5f, 0.5f);
	ctrl_frequency(v);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p5 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p5 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->aeg);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("p5 note off v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->aeg);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	return adsr_is_active(&vs->aeg);
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;

	float am[n];
	float fm[n];

	// generate the modulator
	sin_gen(&vs->modulator, fm, NULL, n);
	block_mul_k(fm, ps->modulator_level, n);

	// generate the carrier
	sin_gen(&vs->carrier, out_l, fm, n);

	// generate the output envelope
	adsr_gen(&vs->aeg, am, n);
	block_mul_k(am, 0.5f, n);

	// apply the envelope
	block_mul(out_l, am, n);

	// copy to the right channel
	block_copy(out_r, out_l, n);

}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));
	ps->modulator_level = 1.0;
	ps->modulator_factor = 1.0;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p5 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:
		ps->modulator_level = midi_map(val, 0.f, 400.f);
		break;
	case 2:
		ps->modulator_factor = midi_map(val, 1.0f, 32.f);
		update = 1;
		break;
	default:
		break;
	}

	if (update) {
		// update each voice using this patch
		for (int i = 0; i < NUM_VOICES; i++) {
			struct voice *v = &p->ggm->voices[i];
			if (v->patch == p) {
				ctrl_frequency(v);
			}
		}
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p5 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	// update each voice using this patch
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &p->ggm->voices[i];
		if (v->patch == p) {
			ctrl_frequency(v);
		}
	}
}

//-----------------------------------------------------------------------------

const struct patch_ops patch5 = {
	.start = start,
	.stop = stop,
	.note_on = note_on,
	.note_off = note_off,
	.active = active,
	.generate = generate,
	.init = init,
	.control_change = control_change,
	.pitch_wheel = pitch_wheel,
};

//-----------------------------------------------------------------------------
