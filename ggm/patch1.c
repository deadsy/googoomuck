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
	struct pan pan;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
	float duty;		// duty cycle for gwave (0..1)
	float slope;		// slope for gwave (0..1)
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	gwave_ctrl_frequency(&vs->gwave, midi_to_frequency((float)v->note + ps->bend));
}

static void ctrl_shape(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	gwave_ctrl_shape(&vs->gwave, ps->duty, ps->slope);
}

static void ctrl_pan(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	pan_ctrl(&vs->pan, ps->vol, ps->pan);
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p1 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));

	adsr_init(&vs->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
	gwave_init(&vs->gwave);
	pan_init(&vs->pan);

	ctrl_frequency(v);
	ctrl_shape(v);
	ctrl_pan(v);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p1 stop (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p1 note on (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->adsr);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("p1 note off (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_release(&vs->adsr);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	return adsr_is_active(&vs->adsr);
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	struct v_state *vs = (struct v_state *)v->state;
	float am[n];
	float out[n];
	// generate the envelope
	adsr_gen(&vs->adsr, am, n);
	// generate the gwave
	gwave_gen(&vs->gwave, out, NULL, n);
	// apply the envelope
	block_mul(out, am, n);
	// pan to left/right channels
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	ps->vol = 1.f;
	ps->pan = 0.5f;
	ps->duty = 0.5f;
	ps->slope = 0.5f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p1 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:		// volume
		ps->vol = midi_map(val, 0.f, 1.5f);
		update = 1;
		break;
	case 2:		// left/right pan
		ps->pan = midi_map(val, 0.f, 1.f);
		update = 1;
		break;
	case 5:
		ps->duty = midi_map(val, 0.f, 1.f);
		update = 2;
		break;
	case 6:
		ps->slope = midi_map(val, 0.f, 1.f);
		update = 2;
		break;
	default:
		break;
	}
	if (update == 1) {
		update_voices(p, ctrl_pan);
	}
	if (update == 2) {
		update_voices(p, ctrl_shape);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p1 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	update_voices(p, ctrl_frequency);
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
