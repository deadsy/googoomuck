//-----------------------------------------------------------------------------
/*

Left/Right Panning and Output Volume Setting

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

void pan_gen(struct pan *p, float *out_l, float *out_r, const float *in, size_t n) {
	block_copy_mul_k(out_l, in, p->vol_l, n);
	block_copy_mul_k(out_r, in, p->vol_r, n);
}

void pan_ctrl(struct pan *p, float vol, float pan) {
	// convert to a linear volume
	vol = pow2(vol) - 1.f;
	// Use sin/cos so that l*l + r*r = K (constant power)
	pan *= PI / 2.f;
	p->vol_l = vol * cos_eval(pan);
	p->vol_r = vol * sin_eval(pan);
}

void pan_init(struct pan *p) {
	// do nothing
}

//-----------------------------------------------------------------------------
