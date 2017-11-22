//-----------------------------------------------------------------------------
/*

Patch One

An ADSR envelope on a goom wave.

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct gwave gwave;
	struct adsr adsr;
};

struct p_state {
	float duty;		// duty cycle for gwave (0..1)
	float slope;		// slope for gwave (0..1)
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("patch1 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	memset(vs, 0, sizeof(struct v_state));
	// setup the gwave
	gwave_init(&vs->gwave, 1.f, midi_to_frequency(v->note), 0.f);
	gwave_shape(&vs->gwave, ps->duty, ps->slope);
	// setup the adsr
	adsr_init(&vs->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("patch1 stop (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("patch1 note on (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->adsr);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("patch1 note off (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->adsr);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	return adsr_is_active(&vs->adsr);
}

// generate samples
static void generate(struct voice *v, float *out, size_t n) {
	float am[n];
	struct v_state *vs = (struct v_state *)v->state;
	adsr_gen(&vs->adsr, am, n);
	gwave_gen_am(&vs->gwave, out, am, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));
	ps->duty = 0.5;
	ps->slope = 1.0;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("patch1 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:
		ps->duty = (float)val / 127.f;
		update = 1;
		break;
	case 2:
		ps->slope = (float)val / 127.f;
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
				struct v_state *vs = (struct v_state *)v->state;
				gwave_shape(&vs->gwave, ps->duty, ps->slope);
			}
		}
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	DBG("patch1 pitch %d\r\n", val);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch1 = {
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
