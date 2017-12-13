//-----------------------------------------------------------------------------
/*

Patch X (General Patch Template)

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
};

struct p_state {
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("pX start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));
}

// stop the patch
static void stop(struct voice *v) {
	DBG("pX stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("pX note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("pX note off v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	return 0;
}

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	//struct p_state *ps = (struct p_state *)p->state;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	int update = 0;

	DBG("pX ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	default:
		break;
	}
	if (update) {
		// update each voice using this patch
		for (int i = 0; i < NUM_VOICES; i++) {
			struct voice *v = &p->ggm->voices[i];
			if (v->patch == p) {
				// ....
			}
		}
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	DBG("pX pitch %d\r\n", val);
}

//-----------------------------------------------------------------------------

const struct patch_ops patchX = {
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
