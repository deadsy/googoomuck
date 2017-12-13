//-----------------------------------------------------------------------------
/*

Goom Voice - using the original code

*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

// constant to give correct tuning for sample rate
// k = math.log(fs/(440.0*math.pow(2.0,-0.75))/128.0)/math.log(2.0)*4096.0
// fs = 72e6 / 2048 = 35156.25 Hz -> k = 287 (goom original)
// fs = 44099.507 Hz -> k = 1627
#define TUNING_K 1627

//-----------------------------------------------------------------------------

struct egparams {		// envelope generator parameters
	unsigned short a, d, s, r;
};

struct egvars {			// envelope generator variables
	unsigned short state;	// 0=off, 1=attacking, 2=holding, 3=decaying, 4=sustaining, 5=releasing
	unsigned short logout;	// logarithmic output 4Q12
	unsigned short out;	// linear output Q16
};

struct v_state {
	unsigned char fk;	// 0   filter constant
	signed char note;	// 1   note number; b7: note is down
	signed char chan;	// 2   channel to which this voice is allocated
	unsigned char vel;	// 3   note on velocity
	unsigned short o0p;	// 4     oscillator 0 waveshape constants
	unsigned short o0k0, o0k1;	// 6,8
	unsigned short o1p;	// 10    oscillator 1 waveshape constants
	unsigned short o1k0, o1k1;	// 12,14
	unsigned short o1egstate;	// 16    oscillator 1 envelope generator state
	unsigned short o1eglogout;	// 18    oscillator 1 envelope generator logarithmic output
	unsigned short vol;	// 20    current output level, copied from amplitude envelope generator output on zero-crossings
	unsigned short eg0trip;	// 22    count of update cycles since volume update
	struct egvars egv[2];	// 24    variables for amplitude and filter envelope generators
	unsigned short lvol, rvol;	// 36,38 multipliers for L and R outputs
	unsigned short o1egout;	// 40    oscillator 1 envelope generator linear output
	unsigned short o1vol;	// 42    current oscillator 1 level, copied from amplitude envelope generator output on zero-crossings
	int o0ph;		// 44    oscillator 0 phase accumulator
	int o1ph;		// 48    oscillator 1 phase accumulator
	int o0dph;		// 52    oscillator 0 phase increment
	int o1dph;		// 56    oscillator 1 phase increment
	int o1fb;		// 60    oscillator 1 feedback value
	int lo, ba;		// 64,68 filter state
	int out;		// 72    last output sample from this voice
	int o1o;		// 76    oscillator 1 output
};

struct p_state {
	unsigned short o1ega, o1egd;	// 0,2   oscillator 1 envelope generator parameters
	unsigned short o1vol;	// 4     oscillator 1 output level
	unsigned short lvol, rvol;	// 6,8   pan values
	unsigned char cut;	// 10    filter cutoff
	signed char fega;	// 11    filter sensitivity
	unsigned char res;	// 12    filter resonance
	unsigned char omode;	// 13    oscillator mode: 0=mix 1=FM 2=FM+FB
	struct egparams egp[2];	// 14,22 parameters for amplitude and filter envelope generators

	// moved into patch state
	unsigned char ctrl[24];	// 7-bit control values
	unsigned char sus;	// sustain pedal position
	short pbend;		// pitch bend position
};

_Static_assert(sizeof(struct v_state) <= VOICE_STATE_SIZE, "sizeof(struct v_state) > VOICE_STATE_SIZE");
_Static_assert(sizeof(struct p_state) <= PATCH_STATE_SIZE, "sizeof(struct p_state) > PATCH_STATE_SIZE");

//-----------------------------------------------------------------------------

const short sintab[256] = {	// sine table, linearly interpolated by oscillators:
// In Octave:
// a=round(sin(2*pi*([-63 -63:1:63 63]/252))*32767)
// reshape([a'(1:128) a'(2:129)-a'(1:128)]',1,256)

	-32767, 0, -32767, 10, -32757, 31, -32726, 51, -32675, 71, -32604, 91, -32513, 112, -32401, 132, -32269, 152, -32117, 172,
	-31945, 191, -31754, 212, -31542, 231, -31311, 250, -31061, 270, -30791, 289, -30502, 308, -30194, 327, -29867, 345, -29522, 364,
	-29158, 381, -28777, 400, -28377, 417, -27960, 435, -27525, 452, -27073, 468, -26605, 485, -26120, 502, -25618, 517, -25101, 533,
	-24568, 548, -24020, 563, -23457, 578, -22879, 592, -22287, 606, -21681, 619, -21062, 632, -20430, 645, -19785, 657, -19128, 670,
	-18458, 680, -17778, 692, -17086, 703, -16383, 712, -15671, 722, -14949, 732, -14217, 740, -13477, 749, -12728, 757, -11971, 764,
	-11207, 771, -10436, 778, -9658, 783, -8875, 790, -8085, 794, -7291, 798, -6493, 803, -5690, 806, -4884, 810, -4074, 811,
	-3263, 814, -2449, 816, -1633, 816, -817, 817, 0, 817, 817, 816, 1633, 816, 2449, 814, 3263, 811, 4074, 810,
	4884, 806, 5690, 803, 6493, 798, 7291, 794, 8085, 790, 8875, 783, 9658, 778, 10436, 771, 11207, 764, 11971, 757,
	12728, 749, 13477, 740, 14217, 732, 14949, 722, 15671, 712, 16383, 703, 17086, 692, 17778, 680, 18458, 670, 19128, 657,
	19785, 645, 20430, 632, 21062, 619, 21681, 606, 22287, 592, 22879, 578, 23457, 563, 24020, 548, 24568, 533, 25101, 517,
	25618, 502, 26120, 485, 26605, 468, 27073, 452, 27525, 435, 27960, 417, 28377, 400, 28777, 381, 29158, 364, 29522, 345,
	29867, 327, 30194, 308, 30502, 289, 30791, 270, 31061, 250, 31311, 231, 31542, 212, 31754, 191, 31945, 172, 32117, 152,
	32269, 132, 32401, 112, 32513, 91, 32604, 71, 32675, 51, 32726, 31, 32757, 10, 32767, 0,
};

// product of the following two tables is exp_2 of 12-bit fraction in Q30
static const unsigned short exptab0[64] = {	// "top octave generator": round(2^15*(2.^([0:1:63]/64)))
	32768, 33125, 33486, 33850, 34219, 34591, 34968, 35349, 35734, 36123, 36516, 36914, 37316, 37722, 38133, 38548,
	38968, 39392, 39821, 40255, 40693, 41136, 41584, 42037, 42495, 42958, 43425, 43898, 44376, 44859, 45348, 45842,
	46341, 46846, 47356, 47871, 48393, 48920, 49452, 49991, 50535, 51085, 51642, 52204, 52773, 53347, 53928, 54515,
	55109, 55709, 56316, 56929, 57549, 58176, 58809, 59449, 60097, 60751, 61413, 62081, 62757, 63441, 64132, 64830
};

static const unsigned short exptab1[64] = {	// fine tuning: round(2^15*(2.^([0:1:31]/1024)))
	32768, 32774, 32779, 32785, 32790, 32796, 32801, 32807, 32812, 32818, 32823, 32829, 32835, 32840, 32846, 32851,
	32857, 32862, 32868, 32874, 32879, 32885, 32890, 32896, 32901, 32907, 32912, 32918, 32924, 32929, 32935, 32940,
	32946, 32952, 32957, 32963, 32968, 32974, 32979, 32985, 32991, 32996, 33002, 33007, 33013, 33018, 33024, 33030,
	33035, 33041, 33046, 33052, 33058, 33063, 33069, 33074, 33080, 33086, 33091, 33097, 33102, 33108, 33114, 33119
};

//-----------------------------------------------------------------------------

void swvol0(struct v_state *vs) {

	uint16_t r4 = vs->egv[0].out;
	vs->vol = r4;

	uint16_t r2 = vs->lvol;
	uint16_t r3 = vs->rvol;

	r2 = r2 * r4;
	r2 >>= 16;

	r3 = r3 * r4;
	r3 >>= 16;

}

void filter(struct v_state *vs, uint8_t res, int *samples) {
	uint8_t r2 = vs->fk;
	int r4 = vs->lo;
	int r5 = vs->ba;
	for (int i = 0; i < 4; i++) {
		int r7 = r2 * r5;
		r7 >>= 8;
		r7 *= r2;
		r4 += r7 >> 8;
		r7 = res * r5;
		r7 = samples[i] - (r7 >> 8);
		r7 -= r4;
		r7 *= r2;
		r7 >>= 8;
		r7 *= r2;
		r5 += r7 >> 8;
		samples[i] = __SSAT(r4, 17);
	}
	vs->lo = r4;
	vs->ba = r5;
}

void wavupac(struct v_state *vs, struct p_state *ps) {

}

//-----------------------------------------------------------------------------

// waveform generation code
extern void wavupa(struct v_state *vs, struct p_state *ps);

// called once every 4 samples ~9kHz=72MHz/8192
static void CT32B0handler(struct voice *v, int n) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	struct egparams *ep = &ps->egp[n];
	struct egvars *ev = &vs->egv[n];
	int i, j, k;

	wavupa(vs, ps);

	i = ev->logout;
	j = (vs->note & 0x80) || ps->sus != 0;	// note down?
	if (ev->state == 0) {
		i = 0;
	} else {
		if (!j)
			ev->state = 5;	// exit sustain when note is released
		switch (ev->state) {
		case 1:
			i += ep->a;	// attack
			if (i >= 0x10000)
				i = 0xffff, ev->state = 2;
			break;
		case 2:
			i--;	// hold at top of attack
			if (i <= 0xfff0)
				ev->state = 3;	// hold for 16 iterations
			break;
		case 3:
			i -= ep->d;	// decay
			if (i < ep->s)
				i = ep->s, ev->state = 4;
			break;
		case 4:	// sustain
			break;
		case 5:
			i -= ep->r;	// release
			if (i < 0)
				i = 0, ev->state = 0;
			break;
		}
	}

	// output
	ev->logout = i;
	if (i == 0) {
		ev->out = 0;
	} else {
		// compute linear output
		ev->out = (exptab0[(i & 0xfc0) >> 6] * exptab1[i & 0x3f]) >> (31 - (i >> 12));
	}

	if (n == 0) {
		// do oscillator 1 eg as well
		i = vs->eg0trip;
		if (i > 4)
			vs->vol = ev->out;
		if (vs->vol == ev->out)
			i = 0;
		i++;
		vs->eg0trip = i;
		i = vs->o1eglogout;
		if (!j)
			vs->o1egstate = 1;
		if (vs->o1egstate == 0) {
			// attack
			i += ps->o1ega;
			if (i >= 0x10000)
				i = 0xffff, vs->o1egstate = 1;
		} else {
			// decay
			i -= ps->o1egd;
			if (i < 0)
				i = 0;
		}

		// output
		vs->o1eglogout = i;
		if (i == 0) {
			vs->o1egout = 0;
		} else {
			// compute linear output
			vs->o1egout = (((exptab0[(i & 0xfc0) >> 6] * exptab1[i & 0x3f]) >> (31 - (i >> 12))) * ps->o1vol) >> 16;
		}

	} else {
		// recalculate filter coefficient
		k = ((ps->cut * ps->cut) >> 8) + ((vs->egv[1].logout * ((ps->fega * vs->vel) >> 6)) >> 15);
		if (k < 0)
			k = 0;
		if (k > 255)
			k = 255;
		vs->fk = k;
	}
}

//-----------------------------------------------------------------------------

// derive frequency and volume settings from controller values for one voice
static void setfreqvol(struct voice *v, unsigned char *ct) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	int u, l;
	unsigned int f;
	int p, q, p0, p1, p2;

	// oscillator 0 frequency
	u = ((vs->note & 0x7f) << 12) / 12;	// pitch of note, Q12 in octaves, middle C =5
	u += ps->pbend / 12;	// gives +/- 2 semitones
	u -= TUNING_K;
	f = (exptab0[(u & 0xfc0) >> 6] * exptab1[u & 0x3f]) >> (10 - (u >> 12));	// convert to linear frequency
	vs->o0dph = f;

	// oscillator 0 waveform
	l = f >> 13;		// compute slope limit: l=32768 at 1/8 Nyquist
	if (l > 30000)
		l = 30000;	// keep within sensible range
	if (l < 1024)
		l = 1024;

	// waveform has four periods p0-3: slope0, flat0, slope1, flat1
	p = 0x8000 - (ct[16] * 248);	// first half p=[1272,32768]
	q = 0x10000 - p;	// second half q=[32768,64264]
	p0 = (p * (127 - ct[17])) >> 7;
	if (p0 < l)
		p0 = l;		// limit waveform slope
	p1 = p - p0;
	p2 = (q * (127 - ct[17])) >> 7;
	if (p2 < l)
		p2 = l;
//  p3=q-p2; // not used
	vs->o0p = (p0 + p2) / 2 + p1;	// constants used by assembler code
	vs->o0k0 = 0x800000 / p0;
	vs->o0k1 = 0x800000 / p2;

	// oscillator 1 frequency
	if (ct[7] > 0x60)
		u = -0x1000 - TUNING_K;	// fixed "low" frequency
	else if (ct[7] > 0x20)
		u = 0x3000 - TUNING_K;	// fixed "high" frequency
	u += (ct[2] << 7) + (ct[3] << 3) - 0x2200;
	f = (exptab0[(u & 0xfc0) >> 6] * exptab1[u & 0x3f]) >> (10 - (u >> 12));
	vs->o1dph = f;

	// oscillator 1 waveform
	l = f >> 13;		// =32768 at 1/8 Nyquist
	if (l > 30000)
		l = 30000;
	if (l < 1024)
		l = 1024;

	p = 0x8000 - (ct[0] * 248);	// first half p=[1272,32768]
	q = 0x10000 - p;	// second half q=[32768,64264]
	p0 = (p * (127 - ct[1])) >> 7;
	if (p0 < l)
		p0 = l;
	p1 = p - p0;
	p2 = (q * (127 - ct[1])) >> 7;
	if (p2 < l)
		p2 = l;
//  p3=q-p2; // not used
	vs->o1p = (p0 + p2) / 2 + p1;
	vs->o1k0 = 0x800000 / p0;
	vs->o1k1 = 0x800000 / p2;

	vs->lvol = (ps->lvol * vs->vel) >> 7;	// calculate output multipliers taking velocity into account
	vs->rvol = (ps->rvol * vs->vel) >> 7;
}

//-----------------------------------------------------------------------------

// 24 controls
// 0  - ctrl 16 - oscillator 1 duty
// 1  - ctrl 17 - oscillator 1 slope
// 2  - ctrl 18 - coarse tuning
// 3  - ctrl 19 - fine tuning
// 4  - ctrl 20 - oscillator 1 envelope attack
// 5  - ctrl 21 - oscillator 1 envelope decay
// 6  - ctrl 22 - oscillator 1 output level
// 7  - ctrl 23 - oscillator 1 frequency mode (3 way switch)
// 8  - ctrl 24 - filter envelope attack
// 9  - ctrl 25 - filter envelope decay
// 10 - ctrl 26 - filter envelope sustain
// 11 - ctrl 27 - filter envelope release
// 12 - ctrl 28 - oscillator 0 envelope attack
// 13 - ctrl 29 - oscillator 0 envelope decay
// 14 - ctrl 30 - oscillator 0 envelope sustain
// 15 - ctrl 31 - oscillator 0 envelope release
// 16 - ctrl 102 - oscillator 0 duty
// 17 - ctrl 103 - oscillator 0 slope
// 18 - ctrl 104 - oscillator combine mode (3 way switch)
// 19 - ctrl 105 - fliter sensitivity
// 20 - ctrl 106 - filter cutoff
// 21 - ctrl 107 - filter resonance
// 22 - ctrl 108 - output volume
// 23 - ctrl 109 - output pan

// process all controllers for given channel
static void procctrl(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	unsigned char *ct = ps->ctrl;
	int i;

	i = ct[6];
	if (i)
		i = (exptab0[(i & 0xf) << 2]) >> (7 - (i >> 4));	// convert oscillator 1 level to linear
	ps->o1vol = i;

	ps->o1ega = 0xffff / (ct[4] * ct[4] / 16 + 1);	// scale oscillator 1 eg parameters
	if (ct[5] == 127)
		ps->o1egd = 0;
	else
		ps->o1egd = 0xffff / (ct[5] * ct[5] + 1);

	ps->egp[0].a = 0xffff / (ct[12] * ct[12] / 16 + 1);	// scale amplitude eg parameters
	ps->egp[0].d = 0xffff / (ct[13] * ct[13] + 1);
	if (ct[14] == 0)
		ps->egp[0].s = 0;
	else
		ps->egp[0].s = 0xc000 + (ct[14] << 7);
	ps->egp[0].r = 0xffff / (ct[15] * ct[15] + 1);

	ps->egp[1].a = 0xffff / (ct[8] * ct[8] / 16 + 1);	// scale filter eg parameters
	ps->egp[1].d = 0xffff / (ct[9] * ct[9] + 1);
	i = ct[10];		// sustain level
	if (i)
		i = exptab0[(i & 0xf) << 2] >> (7 - (i >> 4));
	ps->egp[1].s = i;
	ps->egp[1].r = 0xffff / (ct[11] * ct[11] + 1);

	ps->cut = ct[20] << 1;	// scale filter control parameters
	ps->fega = (ct[19] << 1) - 128;
	ps->res = 0xff - (ct[21] << 1);

	if (ct[18] < 0x20)
		ps->omode = 0;	// oscillator combine mode
	else if (ct[18] > 0x60)
		ps->omode = 2;
	else
		ps->omode = 1;

	i = ct[22];		// volume
	if (i)
		i = exptab0[(i & 0xf) << 2] >> (7 - (i >> 4));	// convert to linear
	ps->lvol = (sintab[254 - (ct[23] & ~1)] * i) >> 15;	// apply pan settings maintining constant total power
	ps->rvol = (sintab[128 + (ct[23] & ~1)] * i) >> 15;

	// update each voice using this patch
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &p->ggm->voices[i];
		if (v->patch == p) {
			setfreqvol(v, ct);
		}
	}
}

//-----------------------------------------------------------------------------
// voice operations

// start the patch
static void start(struct voice *v) {
	DBG("p4 start v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	memset(vs, 0, sizeof(struct v_state));
}

// stop the patch
static void stop(struct voice *v) {
	DBG("p4 stop v%d c%d n%d\r\n", v->idx, v->channel, v->note);
}

// note on
static void note_on(struct voice *v, uint8_t vel) {
	DBG("p4 note on v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	vs->note = v->note | 0x80;	// set up voice data
	vs->chan = v->channel;
	vs->vel = vel;
	setfreqvol(v, ps->ctrl);
	vs->egv[0].state = 1;	// trigger note
	vs->egv[1].state = 1;
	vs->eg0trip = 0;
	vs->o1egstate = 0;
	vs->o1eglogout = 0;
	vs->o1egout = 0;
	vs->out = 0;
}

// note off
static void note_off(struct voice *v, uint8_t vel) {
	DBG("p4 note off v%d c%d n%d\r\n", v->idx, v->channel, v->note);
	struct v_state *vs = (struct v_state *)v->state;
	vs->note = v->note & 0x7f;
}

// return !=0 if the patch is active
static int active(struct voice *v) {
	struct v_state *vs = (struct v_state *)v->state;
	return vs->egv[0].state != 0;
}

// L,R samples being prepared
int tbuf[4][2];

// generate samples
static void generate(struct voice *v, float *out_l, float *out_r, size_t n) {
	for (size_t i = 0; i < n; i += 4) {
		CT32B0handler(v, (i >> 2) & 1);
		out_l[i + 0] = q31_to_float(tbuf[0][0]);
		out_l[i + 1] = q31_to_float(tbuf[1][0]);
		out_l[i + 2] = q31_to_float(tbuf[2][0]);
		out_l[i + 3] = q31_to_float(tbuf[3][0]);
		out_r[i + 0] = q31_to_float(tbuf[0][1]);
		out_r[i + 1] = q31_to_float(tbuf[1][1]);
		out_r[i + 2] = q31_to_float(tbuf[2][1]);
		out_r[i + 3] = q31_to_float(tbuf[3][1]);
	}
}

//-----------------------------------------------------------------------------
// global operations

static void init(struct patch *p) {
	struct p_state *ps = (struct p_state *)p->state;
	// default all the controls to midway (64)
	memset(ps->ctrl, 64, sizeof(ps->ctrl));
	procctrl(p);
}

static void control_change(struct patch *p, uint8_t ctrl, uint8_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	int update = 0;
	DBG("p4 ctrl %d val %d\r\n", ctrl, val);
	if (ctrl == 64) {
		ps->sus = val;
		update = 1;
	} else if (ctrl >= 16 && ctrl < 32) {
		ps->ctrl[ctrl - 16] = val;
		update = 1;
	} else if (ctrl >= 102 && ctrl < 110) {
		ps->ctrl[ctrl - 102 + 16] = val;
		update = 1;
	}
	if (update) {
		procctrl(p);
	}
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	struct p_state *ps = (struct p_state *)p->state;
	DBG("p4 pitch %d\r\n", val);
	ps->pbend = val - 0x2000;
	// update each voice using this patch
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &p->ggm->voices[i];
		if (v->patch == p) {
			setfreqvol(v, ps->ctrl);
		}
	}
}

//-----------------------------------------------------------------------------

const struct patch_ops patch4 = {
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
