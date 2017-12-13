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
	struct pan pan;		// output volume and panning
	float velocity;		// note velocity 0..1
};

enum {
	OMODE_MIX,		// mix osc0 and osc1
	OMODE_FM,		// osc1 FMs osc0
	OMODE_FM_FB,		// feedback on osc1, and osc1 FMs osc0
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend
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
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency_o0(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	gwave_ctrl_frequency(&vs->o0, midi_to_frequency((float)v->note + ps->bend));
}

static void ctrl_shape_o0(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	gwave_ctrl_shape(&vs->o0, ps->o0_duty, ps->o0_slope);
}

static void ctrl_frequency_o1(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	float note = (ps->f_mode) ? ps->f_mode : v->note;
	// TODO tuning
	gwave_ctrl_frequency(&vs->o1, midi_to_frequency((float)note + ps->bend));
}

static void ctrl_shape_o1(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	gwave_ctrl_shape(&vs->o1, ps->o1_duty, ps->o1_slope);
}

static void ctrl_lpf(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	svf_ctrl_cutoff(&vs->lpf, ps->cutoff);
	svf_ctrl_resonance(&vs->lpf, ps->resonance);
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
	struct v_state *vs = (struct v_state *)v->state;
	DBG("p3 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	memset(vs, 0, sizeof(struct v_state));

	// setup oscillator 0
	gwave_init(&vs->o0);
	ctrl_frequency_o0(v);
	ctrl_shape_o0(v);

	// setup oscillator 1
	gwave_init(&vs->o1);
	ctrl_frequency_o1(v);
	ctrl_shape_o1(v);

	// setup the filter
	svf_init(&vs->lpf);
	ctrl_lpf(v);
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
	float out[n];

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
	// TODO use buf1 for the RT cutoff
	svf_gen(&vs->lpf, out, buf0, n);
	// out has the filter output

	// generate the envelope
	adsr_gen(&vs->aeg, buf0, n);
	block_mul_k(buf0, vs->velocity, n);

	// apply the envelope
	block_mul(out, buf0, n);

	// pan to left/right channels
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;

	ps->bend = 0.f;
	ps->vol = 0.3f;
	ps->pan = 0.5f;

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

}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p3 ctrl %d val %d\r\n", ctrl, val);

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
		ps->o1_level = midi_map(val, 5.f, 40.f);
		break;
	case 6:
		ps->eg_a = midi_map(val, 0.01f, 1.f);
		break;
	case 7:
		ps->eg_d = midi_map(val, 0.05f, 5.f);
		break;
	default:
		break;
	}

	if (update == 1) {
		update_voices(p, ctrl_pan);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p3 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	// update each voice using this patch
	//,,,
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
