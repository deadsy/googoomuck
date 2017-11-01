//-----------------------------------------------------------------------------
/*

DDS Lookup Table Based Oscillators

*/
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

#define COS_TABLE_BITS 6U
#define COS_TABLE_SIZE (1U << COS_TABLE_BITS)

static const uint32_t cos_table[COS_TABLE_SIZE] = {
	0x3f800000U, 0x3f7ec46dU, 0x3f7b14beU, 0x3f74fa0bU,
	0x3f6c835eU, 0x3f61c598U, 0x3f54db31U, 0x3f45e403U,
	0x3f3504f3U, 0x3f226799U, 0x3f0e39daU, 0x3ef15aeaU,
	0x3ec3ef15U, 0x3e94a031U, 0x3e47c5c2U, 0x3dc8bd36U,
	0x248d3132U, 0xbdc8bd36U, 0xbe47c5c2U, 0xbe94a031U,
	0xbec3ef15U, 0xbef15aeaU, 0xbf0e39daU, 0xbf226799U,
	0xbf3504f3U, 0xbf45e403U, 0xbf54db31U, 0xbf61c598U,
	0xbf6c835eU, 0xbf74fa0bU, 0xbf7b14beU, 0xbf7ec46dU,
	0xbf800000U, 0xbf7ec46dU, 0xbf7b14beU, 0xbf74fa0bU,
	0xbf6c835eU, 0xbf61c598U, 0xbf54db31U, 0xbf45e403U,
	0xbf3504f3U, 0xbf226799U, 0xbf0e39daU, 0xbef15aeaU,
	0xbec3ef15U, 0xbe94a031U, 0xbe47c5c2U, 0xbdc8bd36U,
	0xa553c9caU, 0x3dc8bd36U, 0x3e47c5c2U, 0x3e94a031U,
	0x3ec3ef15U, 0x3ef15aeaU, 0x3f0e39daU, 0x3f226799U,
	0x3f3504f3U, 0x3f45e403U, 0x3f54db31U, 0x3f61c598U,
	0x3f6c835eU, 0x3f74fa0bU, 0x3f7b14beU, 0x3f7ec46dU,
};

//-----------------------------------------------------------------------------

static void block_mul(float *out, float *k, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		out[i] *= k[i];
	}
}

//-----------------------------------------------------------------------------

static void dds_init(struct dds *osc, float amp, float freq, float phase) {
	memset(osc, 0, sizeof(struct dds));
	// amplitude
	osc->amp = amp;
	// frequency
	osc->freq = freq;
	osc->xstep = (uint32_t) (osc->freq * DDS_FSCALE);
	// phase
	osc->phase = fmodf(phase, TAU);
}

static void dds_table(struct dds *osc, float *table, int bits) {
	// setup the table
	osc->table = table;
	osc->table_mask = (1U << bits) - 1U;
	osc->frac_bits = 32U - bits;
	osc->frac_mask = (1U << osc->frac_bits) - 1;
	osc->frac_scale = (float)(1.f / (float)(1ULL << osc->frac_bits));
}

// simple dds generation (no modulation)
void dds_gen(struct dds *osc, float *out, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		int x0 = osc->x >> osc->frac_bits;
		int x1 = (x0 + 1) & osc->table_mask;
		float y0 = osc->table[x0];
		float y1 = osc->table[x1];
		// interpolate
		out[i] = osc->amp * (y0 + (y1 - y0) * osc->frac_scale * (float)(osc->x & osc->frac_mask));
		// step the x position
		osc->x += osc->xstep;
	}
}

// dds generation with frequency modulation
void dds_gen_fm(struct dds *osc, float *out, float *fm, size_t n) {
	unsigned int i;
	for (i = 0; i < n; i++) {
		int x0 = osc->x >> osc->frac_bits;
		int x1 = (x0 + 1) & osc->table_mask;
		float y0 = osc->table[x0];
		float y1 = osc->table[x1];
		// interpolate
		out[i] = osc->amp * (y0 + (y1 - y0) * osc->frac_scale * (float)(osc->x & osc->frac_mask));
		// step the x position
		osc->x += (uint32_t) ((osc->freq + fm[i]) * DDS_FSCALE);
	}
}

// dds generation with amplitude modulation
void dds_gen_am(struct dds *osc, float *out, float *am, size_t n) {
	dds_gen(osc, out, n);
	block_mul(out, am, n);
}

// dds generation with fm followed by am
void dds_gen_fm_am(struct dds *osc, float *out, float *fm, float *am, size_t n) {
	dds_gen_fm(osc, out, fm, n);
	block_mul(out, am, n);
}

//-----------------------------------------------------------------------------

void dds_sin_init(struct dds *osc, float amp, float freq, float phase) {
	dds_init(osc, amp, freq, phase);
	dds_table(osc, (float *)cos_table, COS_TABLE_BITS);
}

//-----------------------------------------------------------------------------
