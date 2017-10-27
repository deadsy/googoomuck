//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "audio.h"
#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

int ggm_init(struct ggm_state *s, struct ggm_cfg *cfg) {
	int rc = 0;

	memset(s, 0, sizeof(struct ggm_state));
	s->cfg = *cfg;

	rc = event_init();
	if (rc != 0) {
		DBG("event_init failed %d\r\n", rc);
		goto exit;
	}

	osc_sin(&s->sin, 1.0f, midi_to_frequency(69), 0.0f);
	osc_sin(&s->lfo, 20.0f, 4.0f, 0.0f);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------

int ggm_run(struct ggm_state *s) {

	while (1) {
		float x = lut_sample(&s->sin);
		float df = lut_sample(&s->lfo);
		lut_mod_freq(&s->sin, df);
		audio_wr(s->cfg.audio, x, x);
	}

	return 0;
}

//-----------------------------------------------------------------------------
