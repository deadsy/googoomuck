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

#define NCHAN NUM_PATCHES
#define NPOLY NUM_VOICES

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
const unsigned short exptab0[64] = {	// "top octave generator": round(2^15*(2.^([0:1:63]/64)))
	32768, 33125, 33486, 33850, 34219, 34591, 34968, 35349, 35734, 36123, 36516, 36914, 37316, 37722, 38133, 38548,
	38968, 39392, 39821, 40255, 40693, 41136, 41584, 42037, 42495, 42958, 43425, 43898, 44376, 44859, 45348, 45842,
	46341, 46846, 47356, 47871, 48393, 48920, 49452, 49991, 50535, 51085, 51642, 52204, 52773, 53347, 53928, 54515,
	55109, 55709, 56316, 56929, 57549, 58176, 58809, 59449, 60097, 60751, 61413, 62081, 62757, 63441, 64132, 64830
};

const unsigned short exptab1[64] = {	// fine tuning: round(2^15*(2.^([0:1:31]/1024)))
	32768, 32774, 32779, 32785, 32790, 32796, 32801, 32807, 32812, 32818, 32823, 32829, 32835, 32840, 32846, 32851,
	32857, 32862, 32868, 32874, 32879, 32885, 32890, 32896, 32901, 32907, 32912, 32918, 32924, 32929, 32935, 32940,
	32946, 32952, 32957, 32963, 32968, 32974, 32979, 32985, 32991, 32996, 33002, 33007, 33013, 33018, 33024, 33030,
	33035, 33041, 33046, 33052, 33058, 33063, 33069, 33074, 33080, 33086, 33091, 33097, 33102, 33108, 33114, 33119
};

//-----------------------------------------------------------------------------

unsigned char chup[NCHAN];	// channel controls updated?
unsigned char sus[NCHAN];	// sustain pedal position
unsigned short knob[24];	// raw 10-bit MSB-justified ADC results
unsigned char ctrl[NCHAN][24];	// 7-bit control values
short pbend[NCHAN];		// pitch bend position

//-----------------------------------------------------------------------------

// derive frequency and volume settings from controller values for one voice
void setfreqvol(struct voice *v, unsigned char *ct) {
	struct v_state *vs = (struct v_state *)v->state;
	struct p_state *ps = (struct p_state *)v->patch->state;
	int u, l;
	unsigned int f;
	int p, q, p0, p1, p2;

	// oscillator 0 frequency
	u = ((vs->note & 0x7f) << 12) / 12;	// pitch of note, Q12 in octaves, middle C =5
	u += pbend[vs->chan] / 12;	// gives +/- 2 semitones
	u -= 287;		// constant to give correct tuning for sample rate: log((72e6/2048)/(440*2^-0.75)/128)/log(2)*4096 for A=440Hz
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
		u = -0x1000 - 287;	// fixed "low" frequency
	else if (ct[7] > 0x20)
		u = 0x3000 - 287;	// fixed "high" frequency
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
	vs->note = v->note | 0x80;	// set up voice data
	vs->chan = v->channel;
	vs->vel = vel;
	setfreqvol(v, ctrl[v->channel]);
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
	vs->egv[0].state = 0;	// stop aeg processing for voice
	vs->egv[0].logout = 0;
	vs->egv[0].out = 0;	// silence voice
	vs->egv[1].state = 0;	// stop feg processing for voice
	vs->egv[1].logout = 0;
	vs->egv[1].out = 0;
	vs->eg0trip = 0;
	vs->o1egstate = 1;
	vs->o1eglogout = 0;
	vs->o1egout = 0;
	vs->o1vol = 0;
	vs->o1o = 0;
	vs->o1fb = 0;
	vs->fk = 0;
	vs->chan = NCHAN - 1;
	vs->vol = 0;
	vs->out = 0;
	vs->o0ph = 0x00000000;
	vs->o0dph = 0x00000000;
	vs->o1dph = 0x00000000;
	vs->lo = vs->ba = 0;
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
	DBG("p4 ctrl %d val %d\r\n", ctrl, val);
}

static void pitch_wheel(struct patch *p, uint16_t val) {
	DBG("p4 pitch %d\r\n", val);
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
