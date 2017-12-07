//-----------------------------------------------------------------------------
/*

Attack Decay Sustain Release Envelopes

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

enum {
	ADSR_STATE_IDLE,
	ADSR_STATE_ATTACK,
	ADSR_STATE_DECAY,
	ADSR_STATE_SUSTAIN,
	ADSR_STATE_RELEASE,
};

//-----------------------------------------------------------------------------

// We can't reach the target level with the asymptotic rise/fall of exponentials.
// We will change state when we are within level_epsilon of the target level.
#define LEVEL_EPSILON (0.001f)
#define LN_LEVEL_EPSILON (-6.9077553f)	// ln(LEVEL_EPSILON)

// Return a k value to give the exponential rise/fall in the required time.
static float get_k(float t) {
	if (t <= 0) {
		return 1.f;
	}
	return 1.f - powe(LN_LEVEL_EPSILON / (t * AUDIO_FS));
}

//-----------------------------------------------------------------------------

// Enter attack state.
void adsr_attack(struct adsr *e) {
	e->state = ADSR_STATE_ATTACK;
}

// Enter release state.
void adsr_release(struct adsr *e) {
	if (e->state != ADSR_STATE_IDLE) {
		if (e->kr == 1.f) {
			// no release - goto idle
			e->val = 0.f;
			e->state = ADSR_STATE_IDLE;
		} else {
			e->state = ADSR_STATE_RELEASE;
		}
	}
}

// Enter idle state.
void adsr_idle(struct adsr *e) {
	e->val = 0.f;
	e->state = ADSR_STATE_IDLE;
}

// Return non-zero if the adsr is active (!=0).
int adsr_is_active(struct adsr *e) {
	return e->state != ADSR_STATE_IDLE;
}

//-----------------------------------------------------------------------------

// Return a sample value for the ADSR envelope.
static float adsr_sample(struct adsr *e) {
	switch (e->state) {
	case ADSR_STATE_IDLE:
		// idle - do nothing
		break;
	case ADSR_STATE_ATTACK:
		// attack until 1.0 level
		if (e->val < e->d_trigger) {
			e->val += e->ka * (1.f - e->val);
		} else {
			// goto decay state
			e->val = 1.f;
			e->state = ADSR_STATE_DECAY;
		}
		break;
	case ADSR_STATE_DECAY:
		// decay until sustain level
		if (e->val > e->s_trigger) {
			e->val += e->kd * (e->s - e->val);
		} else {
			if (e->s != 0.f) {
				// goto sustain state
				e->val = e->s;
				e->state = ADSR_STATE_SUSTAIN;
			} else {
				// no sustain, goto idle state
				e->val = 0.f;
				e->state = ADSR_STATE_IDLE;
			}
		}
		break;
	case ADSR_STATE_SUSTAIN:
		// sustain - do nothing
		break;
	case ADSR_STATE_RELEASE:
		// release until idle level
		if (e->val > e->i_trigger) {
			e->val += e->kr * (0.f - e->val);
		} else {
			// goto idle state
			e->val = 0.f;
			e->state = ADSR_STATE_IDLE;
		}
		break;
	}
	return e->val;
}

void adsr_gen(struct adsr *e, float *out, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		out[i] = adsr_sample(e);
	}
}

//-----------------------------------------------------------------------------

// ADSR envelope initialisation
// a = attack time in seconds (>= 0)
// d = decay time in seconds (>= 0)
// s = sustain level (0 to 1)
// r = release time in seconds (>= 0)
void adsr_init(struct adsr *e, float a, float d, float s, float r) {
	e->s = s;
	e->ka = get_k(a);
	e->kd = get_k(d);
	e->kr = get_k(r);
	e->d_trigger = 1.f - LEVEL_EPSILON;
	e->s_trigger = s + (1.f - s) * LEVEL_EPSILON;
	e->i_trigger = s * LEVEL_EPSILON;
	e->state = ADSR_STATE_IDLE;
	e->val = 0.f;
}

// AD envelope initialisation
// a = attack time in seconds (>= 0)
// d = decay time in seconds (>= 0)
void ad_init(struct adsr *e, float a, float d) {
	adsr_init(e, a, d, 0.f, 0.f);
}

//-----------------------------------------------------------------------------
