//-----------------------------------------------------------------------------
/*

Left/Right Panning and Output Volume Setting

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

void pan_gen(struct pan *pan, float *out_l, float *out_r, const float *in, size_t n) {
	block_copy_mul_k(out_l, in, pan->vol_l, n);
	block_copy_mul_k(out_r, in, pan->vol_r, n);
}

void pan_init(struct pan *pan) {
	pan->vol_l = 0.5f;
	pan->vol_r = 0.5f;
}

//-----------------------------------------------------------------------------
