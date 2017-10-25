//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "audio.h"
#include "ggm.h"

//-----------------------------------------------------------------------------

int ggm_init(struct ggm_state *s, struct ggm_cfg *cfg) {
	int rc = 0;
	float f = midi_to_frequency(60);

	memset(s, 0, sizeof(struct ggm_state));
	s->cfg = *cfg;

	osc_sin(&s->sin, f, s->cfg.fs);
	osc_sin(&s->lfo, 4.0f, s->cfg.fs);

	return rc;
}

//-----------------------------------------------------------------------------

int ggm_run(struct ggm_state *s) {
	int rc = 0;
	float f = midi_to_frequency(60);

	while (1) {
		float x = lut_sample(&s->sin);
		float delta_f = 30.0f * lut_sample(&s->lfo);
		lut_set_frequency(&s->sin, f + delta_f, s->cfg.fs);
		x *= 20000.0f;
		audio_wr(s->cfg.audio, x);
	}

	return rc;
}

//-----------------------------------------------------------------------------
