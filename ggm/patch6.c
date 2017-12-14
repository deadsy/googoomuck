//-----------------------------------------------------------------------------
/*

Patch 6

A simple patch - Just an envelope on noise.

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct adsr adsr;
	struct noise ns;
	struct pan pan;
	int algo;
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_pan(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	pan_ctrl(&vs->pan, ps->vol, ps->pan);
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	DBG("p6 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	memset(vs, 0, sizeof(struct v_state));

	adsr_init(&vs->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
	noise_init(&vs->ns);
	pan_init(&vs->pan);
	ctrl_pan(v);

	vs->algo = v->note % 4;
	DBG("algo %d\r\n", vs->algo);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p6 stop (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p6 note on (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->adsr);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("p6 note off (%d %d %d)\r\n", v->idx, v->channel, v->note);
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

	// generate the noise
	if (vs->algo == 0) {
		noise_gen_white(&vs->ns, out, n);
	} else if (vs->algo == 1) {
		noise_gen_pink1(&vs->ns, out, n);
	} else if (vs->algo == 2) {
		noise_gen_pink2(&vs->ns, out, n);
	} else if (vs->algo == 3) {
		noise_gen_brown(&vs->ns, out, n);
	}
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
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p6 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:		// volume
		ps->vol = midi_map(val, 0.f, 1.5f);
		update = 1;
		break;
	case 2:		// left/right pan
		ps->pan = midi_map(val, 0.f, 1.f);
		update = 1;
		break;
	default:
		break;
	}
	if (update) {
		update_voices(p, ctrl_pan);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p6 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	//update_voices(p, ctrl_frequency);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch6 = {
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
