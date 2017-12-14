//-----------------------------------------------------------------------------
/*

Noise Generation

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

#if 0
#include <math.h>
#define DEBUG
#include "logging.h"
// record the max output values for scaling determination
static void record_max(struct noise *ns, float x) {
	x = fabs(x);
	if (x > ns->max) {
		ns->max = x;
	}
	ns->count += 1;
	if ((ns->count & ((1 << 13) - 1)) == 0) {
		DBG("max is %08x\r\n", *(uint32_t *) & ns->max);
	}
}
#endif

//-----------------------------------------------------------------------------
// https://noisehack.com/generate-noise-web-audio-api/
// http://www.musicdsp.org/files/pink.txt
// https://en.wikipedia.org/wiki/Pink_noise
// https://en.wikipedia.org/wiki/White_noise
// https://en.wikipedia.org/wiki/White_noise
// https://en.wikipedia.org/wiki/Brownian_noise

void noise_init(struct noise *n) {
	// do nothing
}

// white noise (spectral density = k)
void noise_gen_white(struct noise *ns, float *out, size_t n) {
	for (size_t i = 0; i < n; i++) {
		out[i] = rand_float();
	}
}

// brown noise (spectral density = k/f*f)
void noise_gen_brown(struct noise *ns, float *out, size_t n) {
	float b0 = ns->b0;
	for (size_t i = 0; i < n; i++) {
		float white = rand_float();
		b0 = (b0 + (0.02f * white)) * (1.f / 1.02f);
		out[i] = b0 * (1.f / 0.38f);
	}
	ns->b0 = b0;
}

// pink noise (spectral density = k/f): fast, inaccurate version
void noise_gen_pink1(struct noise *ns, float *out, size_t n) {
	float b0 = ns->b0;
	float b1 = ns->b1;
	float b2 = ns->b2;
	for (size_t i = 0; i < n; i++) {
		float white = rand_float();
		b0 = 0.99765f * b0 + white * 0.0990460f;
		b1 = 0.96300f * b1 + white * 0.2965164f;
		b2 = 0.57000f * b2 + white * 1.0526913f;
		float pink = b0 + b1 + b2 + white * 0.1848f;
		out[i] = pink * (1.f / 10.4f);
	}
	ns->b0 = b0;
	ns->b1 = b1;
	ns->b2 = b2;
}

// pink noise (spectral density = k/f): slow, accurate version
void noise_gen_pink2(struct noise *ns, float *out, size_t n) {
	float b0 = ns->b0;
	float b1 = ns->b1;
	float b2 = ns->b2;
	float b3 = ns->b3;
	float b4 = ns->b4;
	float b5 = ns->b5;
	float b6 = ns->b6;
	for (size_t i = 0; i < n; i++) {
		float white = rand_float();
		b0 = 0.99886f * b0 + white * 0.0555179f;
		b1 = 0.99332f * b1 + white * 0.0750759f;
		b2 = 0.96900f * b2 + white * 0.1538520f;
		b3 = 0.86650f * b3 + white * 0.3104856f;
		b4 = 0.55000f * b4 + white * 0.5329522f;
		b5 = -0.7616f * b5 - white * 0.0168980f;
		float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
		b6 = white * 0.115926f;
		out[i] = pink * (1.f / 10.2f);
	}
	ns->b0 = b0;
	ns->b1 = b1;
	ns->b2 = b2;
	ns->b3 = b3;
	ns->b4 = b4;
	ns->b5 = b5;
	ns->b6 = b6;
}

//-----------------------------------------------------------------------------
