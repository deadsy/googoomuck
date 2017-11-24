//-----------------------------------------------------------------------------
/*

Patch 3

My version of a Goom Voice

See the voice architecture diagram at https://www.quinapalus.com/goom.html

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

struct v_state {
	struct gwave o0;	// main oscillator
	struct gwave o1;	// modulating oscillator
	struct adsr eg;		// envelope generator for oscillator 1
	struct adsr feg;	// filter envelope generator
	struct adsr aeg;	// amplitude envelope generator
	struct lpf lpf;		// output low pass filter
};

enum {
	OMODE_MIX,		// mix osc0 and osc1
	OMODE_FM,		// osc1 FMs osc0
	OMODE_FM_FB,		// feedback on osc1, and osc1 FMs osc0
};

enum {
	FMODE_NOTE,		// osc1 is set to note frequency
	FMODE_HI,		// osc1 is set to high frequency
	FMOD_LO,		// osc1 is set to low frequency
};

struct p_state {
	// oscillator 0
	float o0_duty, o0_slope;	// oscillator 0 duty cycle and slope
	// oscillator 1
	int f_mode;		// frequency mode
	int o_mode;		// oscillator combining mode
	float o1_coarse, o1_fine;	// oscillator 1 frequency adjust
	float o1_duty, o1_slope;	// oscillator 1 duty cycle and slope
	float eg_a, eg_r;	// envelope generator ad parameters
	float o1_level;		// oscillator 1 output level
	// filter
	float feg_a, feg_d, feg_s, feg_r;	// filter envelope generator adsr parameters
	float sensitivity, cutoff, resonance;	// filter controls
	// output
	float aeg_a, aeg_d, aeg_s, aeg_r;	// amplitude envelope generator adsr parameters
	float volume, pan;
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
static void generate(struct voice *v, float *out, size_t n) {
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));
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
