//-----------------------------------------------------------------------------
/*

GooGooMuck Synthesizer

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ggm.h"
#include "led.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------
// key events

// handle a key down event
static void key_dn_handler(struct ggm_state *s, struct event *e) {
	DBG("key down %d\r\n", EVENT_KEY(e->type));
	adsr_attack(&s->adsr);
	//event_wr(EVENT_TYPE_MIDI | MIDI_NOTE_ON(0, 60, 0x40), NULL);
	led_on(LED_BLUE);
}

// handle a key up event
static void key_up_handler(struct ggm_state *s, struct event *e) {
	DBG("key up %d\r\n", EVENT_KEY(e->type));
	adsr_release(&s->adsr);
	//event_wr(EVENT_TYPE_MIDI | MIDI_NOTE_OFF(0, 60, 0x40), NULL);
	led_off(LED_BLUE);
}

//-----------------------------------------------------------------------------
// midi events

// handle a midi event
static void midi_handler(struct ggm_state *s, struct event *e) {
	DBG("midi %06x\r\n", EVENT_MIDI(e->type));
}

//-----------------------------------------------------------------------------
// audio request events

// handle an audio request event
static void audio_handler(struct ggm_state *s, struct event *e) {
	DBG("audio %08x %08x\r\n", e->type, e->ptr);
}

//-----------------------------------------------------------------------------

// the main ggm event loop
int ggm_run(struct ggm_state *s) {

	while (1) {
		struct event e;
		//float a = adsr_sample(&s->adsr);
		//float x = a * lut_sample(&s->sin);

		if (!event_rd(&e)) {
			switch (EVENT_TYPE(e.type)) {
			case EVENT_TYPE_KEY_DN:
				key_dn_handler(s, &e);
				break;
			case EVENT_TYPE_KEY_UP:
				key_up_handler(s, &e);
				break;
			case EVENT_TYPE_MIDI:
				midi_handler(s, &e);
				break;
			case EVENT_TYPE_AUDIO:
				audio_handler(s, &e);
				break;
			default:
				DBG("unknown event %08x %08x\r\n", e.type, e.ptr);
				break;
			}
		}
		//audio_wr(s->audio, x, x);
	}

	return 0;
}

//-----------------------------------------------------------------------------

// initialise the ggm state
int ggm_init(struct ggm_state *s, struct audio_drv *audio) {
	int rc = 0;

	float x = DDS_FSCALE;

	DBG("DDS_FSCALE %08x\r\n", *(uint32_t *) & x);

	memset(s, 0, sizeof(struct ggm_state));
	s->audio = audio;

	rc = event_init();
	if (rc != 0) {
		DBG("event_init failed %d\r\n", rc);
		goto exit;
	}

	osc_sin(&s->sin, 1.0f, midi_to_frequency(69), 0.0f);
	adsr_init(&s->adsr, 0.05f, 0.2f, 0.5f, 0.5f);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
