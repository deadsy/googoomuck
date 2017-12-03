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

#define NOTE_LO 12		// fixed lo frequency note
#define NOTE_HI 36		// fixed hi frequency note

//-----------------------------------------------------------------------------

struct v_state {
	struct gwave o0;	// main oscillator
	struct gwave o1;	// modulating oscillator
	struct adsr eg;		// envelope generator for oscillator 1
	struct adsr feg;	// filter envelope generator
	struct adsr aeg;	// amplitude envelope generator
	struct svf lpf;		// output low pass filter
	float velocity;		// note velocity 0..1
};

enum {
	OMODE_MIX,		// mix osc0 and osc1
	OMODE_FM,		// osc1 FMs osc0
	OMODE_FM_FB,		// feedback on osc1, and osc1 FMs osc0
};

struct p_state {
	// oscillator 0
	float o0_duty, o0_slope;	// oscillator 0 duty cycle and slope
	// oscillator 1
	int f_mode;		// frequency mode (0 == played note, NOTE_LO, NOTE_HI)
	int o_mode;		// oscillator combining mode
	float o1_coarse, o1_fine;	// oscillator 1 frequency adjust
	float o1_duty, o1_slope;	// oscillator 1 duty cycle and slope
	float eg_a, eg_d;	// envelope generator ad parameters
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
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;

	DBG("p3 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	memset(vs, 0, sizeof(struct v_state));

	// setup oscillator 0
	gwave_init(&vs->o0, midi_to_frequency(v->note));
	gwave_shape(&vs->o0, ps->o0_duty, ps->o0_slope);

	// setup oscillator 1
	uint8_t n = (ps->f_mode) ? ps->f_mode : v->note;
	// TODO tuning
	gwave_init(&vs->o1, midi_to_frequency(n));
	gwave_shape(&vs->o1, ps->o0_duty, ps->o0_slope);

	// setup the filter
	svf_init(&vs->lpf, ps->cutoff, ps->resonance);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p3 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;

	DBG("p3 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);

	vs->velocity = (float)vel / 127.f;

	ad_init(&vs->eg, ps->eg_a, ps->eg_d);
	adsr_attack(&vs->eg);

	adsr_init(&vs->feg, ps->feg_a, ps->feg_d, ps->feg_s, ps->feg_r);
	adsr_attack(&vs->feg);

	adsr_init(&vs->aeg, ps->aeg_a, ps->aeg_d, ps->aeg_s, ps->aeg_r);
	adsr_attack(&vs->aeg);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	struct v_state *vs = (struct v_state *)v->state;
	DBG("p3 note off v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	adsr_release(&vs->eg);
	adsr_release(&vs->feg);
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

	float buf0[n];
	float buf1[n];

	// oscillator 1
	if (ps->o_mode == OMODE_FM_FB) {
		// feedback
		// TODO
	} else {
		// no feedback
		adsr_gen(&vs->eg, buf0, n);
		gwave_gen(&vs->o1, buf1, NULL, n);
		block_mul(buf1, buf0, n);
		block_mul_k(buf1, ps->o1_level, n);
	}
	// buf1 has the oscillator 1 output

	// oscillator 0
	if (ps->o_mode == OMODE_MIX) {
		// mix mode
		gwave_gen(&vs->o0, buf0, NULL, n);
		block_add(buf0, buf1, n);
	} else {
		// fm mode
		gwave_gen(&vs->o0, buf0, buf1, n);
	}
	// buf0 has the oscillator 0 output

	// filter
	adsr_gen(&vs->feg, buf1, n);
	block_mul_k(buf1, vs->velocity * ps->sensitivity, n);
	block_add_k(buf1, ps->cutoff, n);
	svf_gen(&vs->lpf, out_l, buf0, buf1, n);
	// out has the filter output

	// output
	adsr_gen(&vs->aeg, buf0, n);
	block_mul_k(buf0, vs->velocity, n);
	block_mul(out_l, buf0, n);

	block_mul_k(out_l, ps->volume, n);
	// TODO pan
	block_copy(out_r, out_l, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	memset(ps, 0, sizeof(struct p_state));

	// oscillator 0
	ps->o0_duty = 0.3f;
	ps->o0_slope = 0.9f;

	// oscillator 1
	ps->f_mode = NOTE_LO;
	ps->o_mode = OMODE_FM;
	ps->o1_coarse = 0.f;
	ps->o1_fine = 0.f;
	ps->o1_duty = 0.5f;
	ps->o1_slope = 0.9f;
	ps->eg_a = 0.05f;
	ps->eg_d = 0.5f;
	ps->o1_level = 5.f;

	// filter
	ps->feg_a = 0.05f;
	ps->feg_d = 0.2f;
	ps->feg_s = 0.5f;
	ps->feg_r = 0.5f;
	ps->sensitivity = 1.f;
	ps->cutoff = 1.f;
	ps->resonance = 1.f;

	// output
	ps->aeg_a = 0.05f;
	ps->aeg_d = 0.2f;
	ps->aeg_s = 0.5f;
	ps->aeg_r = 0.5f;
	ps->volume = 0.3f;
	ps->pan = 0.f;
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	//int update = 0;

	DBG("p3 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {
	case 1:
		ps->o1_level = midi_map(val, 5.f, 40.f);
		break;
	case 2:
		ps->eg_a = midi_map(val, 0.01f, 1.f);
		break;
	case 3:
		ps->eg_d = midi_map(val, 0.05f, 5.f);
		break;
	default:
		break;
	}

#if 0
	if (update) {
		// update each voice using this patch
		for (int i = 0; i < NUM_VOICES; i++) {
			struct voice *v = &p->ggm->voices[i];
			if (v->patch == p) {
				// ....
			}
		}
	}
#endif

}

static void pitch_wheel(struct patch *p, uint16_t val) {
	DBG("pX pitch %d\r\n", val);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch3 = {
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
