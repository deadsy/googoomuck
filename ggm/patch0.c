//-----------------------------------------------------------------------------
/*

Patch Zero

A simple patch - Just an envelope on a sine wave.

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct dds sin;
	struct adsr adsr;
};

struct p_state {
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("patch0 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));
	dds_sin_init(&vs->sin, 1.f, midi_to_frequency(v->note), 0.f);
	adsr_init(&vs->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("patch0 stop (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("patch0 note on (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->adsr);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("patch0 note off (%d %d %d)\r\n", v->idx, v->channel, v->note);
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
	dds_gen_am(&vs->sin, out, am, n);
}

//-----------------------------------------------------------------------------
// global operations

static int init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));
	return 0;
}

static void control_change(uint8_t ctrl, uint8_t val) {
	DBG("patch0 ctrl %d val %d\r\n", ctrl, val);
}

static void pitch_wheel(uint16_t val) {
	DBG("patch0 pitch %d\r\n", val);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch0 = {
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
