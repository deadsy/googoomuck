//-----------------------------------------------------------------------------
/*

Patch 2

Karplus Strong Testing

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct ks ks;
};

struct p_state {
	float attenuate;
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("patch2 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	memset(vs, 0, sizeof(struct v_state));
	// setup the ks
	ks_init(&vs->ks, midi_to_frequency(v->note), ps->attenuate);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("patch2 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("patch2 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	ks_pluck(&vs->ks);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	return 1;
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	struct v_state *vs = (struct v_state *)v->state;
	ks_gen(&vs->ks, out_l, n);
	block_copy(out_r, out_l, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));
	ps->attenuate = 0.99f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("patch2 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:
		ps->attenuate = midi_map(val, 0.87f, 1.f);
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
				ks_attenuate(&vs->ks, ps->attenuate);
			}
		}
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
}

//-----------------------------------------------------------------------------

const struct patch_ops patch2 = {
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
