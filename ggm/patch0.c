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

struct p_state {
	struct dds sin;
	struct adsr adsr;
};

_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------

// start the patch
static void start(struct voice *v) {
	DBG("patch0 start (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct p_state *p = (struct p_state *)v->state;
	memset(p, 0, sizeof(struct p_state));
	dds_sin_init(&p->sin, 1.f, midi_to_frequency(v->note), 0.f);
	adsr_init(&p->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("patch0 stop (%d %d %d)\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("patch0 note on (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct p_state *p = (struct p_state *)v->state;
	adsr_attack(&p->adsr);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("patch0 note off (%d %d %d)\r\n", v->idx, v->channel, v->note);
	struct p_state *p = (struct p_state *)v->state;
	adsr_release(&p->adsr);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	struct p_state *p = (struct p_state *)v->state;
	return adsr_is_active(&p->adsr);
}

// generate samples
static void generate(struct voice *v, float *out, size_t n) {
	float am[n];
	struct p_state *p = (struct p_state *)v->state;
	adsr_gen(&p->adsr, am, n);
	dds_gen_am(&p->sin, out, am, n);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch0 = {
	.start = start,
	.stop = stop,
	.note_on = note_on,
	.note_off = note_off,
	.active = active,
	.generate = generate,
};

//-----------------------------------------------------------------------------
