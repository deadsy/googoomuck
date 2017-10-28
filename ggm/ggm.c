//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer

*/
//-----------------------------------------------------------------------------

#include <string.h>

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
	osc_sin(&s->lfo, 20.0f, 2.0f, 0.0f);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
// key events

// handle a key down event
static void key_dn_handler(struct event *e) {
	DBG("key down %d\r\n", EVENT_KEY(e->type));
	event_wr(EVENT_TYPE_MIDI | MIDI_NOTE_ON(0, 60, 0x40), NULL);
}

// handle a key up event
static void key_up_handler(struct event *e) {
	DBG("key up %d\r\n", EVENT_KEY(e->type));
	event_wr(EVENT_TYPE_MIDI | MIDI_NOTE_OFF(0, 60, 0x40), NULL);
}

//-----------------------------------------------------------------------------
// midi events

// handle a midi event
static void midi_handler(struct event *e) {
	DBG("midi %06x\r\n", EVENT_MIDI(e->type));
}

//-----------------------------------------------------------------------------

// the main ggm event loop
int ggm_run(struct ggm_state *s) {

	while (1) {
		struct event e;
		float x = lut_sample(&s->sin);
		float df = lut_sample(&s->lfo);

		if (!event_rd(&e)) {
			switch (EVENT_TYPE(e.type)) {
			case EVENT_TYPE_KEY_DN:
				key_dn_handler(&e);
				break;
			case EVENT_TYPE_KEY_UP:
				key_up_handler(&e);
				break;
			case EVENT_TYPE_MIDI:
				midi_handler(&e);
				break;
			default:
				break;
			}
		}

		lut_mod_freq(&s->sin, df);
		audio_wr(s->cfg.audio, x, x);
	}

	return 0;
}

//-----------------------------------------------------------------------------
