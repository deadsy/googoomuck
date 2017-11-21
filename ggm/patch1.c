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

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("patch1 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));
	gwave_init(&vs->gwave, 0.5f, 0.1f, 1.f, midi_to_frequency(v->note), 0.f);
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

static int init(void) {
	// nothing
	return 0;
}

static void control_change(uint8_t ctrl, uint8_t val) {
	DBG("patch1 ctrl %d val %d\r\n", ctrl, val);
}

static void pitch_wheel(uint16_t val) {
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
