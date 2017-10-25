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
	uint8_t notes[3];
	int rc = 0;

	memset(s, 0, sizeof(struct ggm_state));
	s->cfg = *cfg;

	major_chord(notes, 60);

	osc_sin(&s->sin0, midi_to_frequency(notes[0]), s->cfg.fs);
	osc_sin(&s->sin1, midi_to_frequency(notes[1]), s->cfg.fs);
	osc_sin(&s->sin2, midi_to_frequency(notes[2]), s->cfg.fs);

	return rc;
}

//-----------------------------------------------------------------------------

int ggm_run(struct ggm_state *s) {
	int rc = 0;

	while (1) {
		float x = lut_sample(&s->sin0);
		x += lut_sample(&s->sin1);
		x += lut_sample(&s->sin2);
		x *= 10000.0f;
		audio_wr(s->cfg.audio, x);
	}

	return rc;
}

//-----------------------------------------------------------------------------
