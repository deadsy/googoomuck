//-----------------------------------------------------------------------------
/*

Patch 5 - FM Synthesis

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

#define NOTE_LO 12.f		// fixed lo frequency note
#define NOTE_HI 36.f		// fixed hi frequency note

//-----------------------------------------------------------------------------

struct v_state {
	struct sin modulator;
	struct sin carrier;
	struct svf2 lpf;
	struct adsr aeg;
	struct pan pan;
	float fm_level;		// modulator amplitude
};

struct p_state {
	float vol;		// volume
	float pan;		// left/right pan
	float bend;		// pitch bend

	float f_mode;		// modulator frequency mode (hi,lo,note)
	float fm_tune;		// tuning factor for modulator frequency
	float fm_level;		// modulator amplitude

	float cutoff;		// lpf frequency cutoff
	float resonance;	// lpf resonance

};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------
// control functions

static void ctrl_frequency(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	float freq;

	// set the modulator frequency
	float fm_note = (ps->f_mode) ? ps->f_mode : (float)v->note + ps->bend;
	fm_note *= ps->fm_tune;
	freq = midi_to_frequency(fm_note);
	sin_ctrl_frequency(&vs->modulator, freq);
	vs->fm_level = freq * ps->fm_level;

	// set the carrier frequency
	freq = midi_to_frequency((float)v->note + ps->bend);
	sin_ctrl_frequency(&vs->carrier, freq);
}

static void ctrl_lpf(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	float freq = midi_to_frequency((float)v->note + ps->bend);
	svf2_ctrl_cutoff(&vs->lpf, ps->cutoff * freq);
	svf2_ctrl_resonance(&vs->lpf, ps->resonance);
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
	DBG("p5 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	memset(vs, 0, sizeof(struct v_state));
	sin_init(&vs->modulator);
	sin_init(&vs->carrier);
	svf2_init(&vs->lpf);
	adsr_init(&vs->aeg, 0.05f, 0.2f, 0.5f, 0.5f);
	pan_init(&vs->pan);
	ctrl_lpf(v);
	ctrl_frequency(v);
	ctrl_pan(v);
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p5 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p5 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	adsr_attack(&vs->aeg);
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("p5 note off v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
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
	//struct p_state *ps = (struct p_state *)v->patch->state;

	float buf0[n];
	float buf1[n];
	float *fm = buf0;
	float *cout = buf1;
	float *out = buf0;
	float *am = buf1;

	// generate the modulator
	sin_gen(&vs->modulator, fm, NULL, n);
	block_mul_k(fm, vs->fm_level, n);

	// generate the carrier
	sin_gen(&vs->carrier, cout, fm, n);

	// low pass filter
	//svf2_gen(&vs->lpf, out, cout, n);
	block_copy(out, cout, n);

	// apply the output envelope
	adsr_gen(&vs->aeg, am, n);
	block_mul(out, am, n);

	// pan to left/right channels
	pan_gen(&vs->pan, out_l, out_r, out, n);
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;

	ps->vol = 1.f;		// volume
	ps->pan = 0.5f;		// left/right pan
	ps->bend = 0.f;		// pitch bend

	ps->f_mode = NOTE_HI;	//0.f; // modulator frequency mode (hi,lo,note)
	ps->fm_tune = 1.f;	// tuning factor for modulator frequency
	ps->fm_level = 0.1f;	// modulation level

	ps->cutoff = 5.f;	// lpf frequency cutoff
	ps->resonance = 0.5f;	// lpf resonance
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;

	DBG("p5 ctrl %d val %d\r\n", ctrl, val);

	switch (ctrl) {

		// Output
	case 1:		// volume
		ps->vol = midi_map(val, 0.f, 1.5f);
		update = 1;
		break;
	case 2:		// left/right pan
		ps->pan = midi_map(val, 0.f, 1.f);
		update = 1;
		break;

		// FM modulation
	case 5:
		ps->fm_tune = midi_map(val, 0.3f, 1.f / 0.3f);
		update = 2;
		break;
	case 6:
		ps->fm_level = midi_map(val, 0.00f, 5.f);
		update = 2;
		break;

		// LPF response
	case 7:
		ps->cutoff = midi_map(val, 0.5f, 10.f);
		update = 3;
		break;
	case 8:
		ps->resonance = midi_map(val, 0.f, 1.f);
		update = 3;
		break;
	default:
		break;
	}
	if (update == 1) {
		update_voices(p, ctrl_pan);
	}
	if (update == 2) {
		update_voices(p, ctrl_frequency);
	}
	if (update == 3) {
		update_voices(p, ctrl_lpf);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p5 pitch %d\r\n", val);
	ps->bend = midi_pitch_bend(val);
	update_voices(p, ctrl_frequency);
}

//-----------------------------------------------------------------------------

const struct patch_ops patch5 = {
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
