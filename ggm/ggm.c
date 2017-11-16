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
	led_on(LED_BLUE);
}

// handle a key up event
static void key_up_handler(struct ggm_state *s, struct event *e) {
	DBG("key up %d\r\n", EVENT_KEY(e->type));
	adsr_release(&s->adsr);
	led_off(LED_BLUE);
}

//-----------------------------------------------------------------------------
// midi events

// Currently unused.
// Serial MIDI events are handled as they are read from the buffer.
// MIDI events from USB might use this....

// handle a midi event
static void midi_handler(struct ggm_state *s, struct event *e) {
	DBG("midi %06x\r\n", EVENT_MIDI(e->type));
}

//-----------------------------------------------------------------------------
// audio request events

// handle an audio request event
static void audio_handler(struct ggm_state *s, struct event *e) {
	size_t n = EVENT_BLOCK_SIZE(e->type);
	int16_t *dst = e->ptr;
	float out[n];

	//DBG("audio %08x %08x\r\n", e->type, e->ptr);

	if (adsr_is_active(&s->adsr)) {
		//float fm[n];
		float am[n];
		//dds_gen(&s->lfo, fm, n);
		adsr_gen(&s->adsr, am, n);
		gwave_gen_am(&s->gw, out, am, n);
	} else {
		memset(out, 0, n * sizeof(float));
	}

	// write the samples to the dma buffer
	audio_wr(dst, n, out, out);
	// record some realtime stats
	audio_stats(s->audio, dst);
}

//-----------------------------------------------------------------------------

// the main ggm event loop
int ggm_run(struct ggm_state *s) {
	while (1) {
		struct event e;
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
		// get and process serial midi messages
		midi_process(s->midi);
	}
	return 0;
}

//-----------------------------------------------------------------------------

// initialise the ggm state
int ggm_init(struct ggm_state *s, struct audio_drv *audio, struct midi_drv *midi) {
	int rc = 0;

	memset(s, 0, sizeof(struct ggm_state));
	s->audio = audio;
	s->midi = midi;

	rc = event_init();
	if (rc != 0) {
		DBG("event_init failed %d\r\n", rc);
		goto exit;
	}

	dds_sin_init(&s->lfo, 10.f, 15.f, 0.f);
	dds_sin_init(&s->sin, 1.f, midi_to_frequency(69), 0.f);
	adsr_init(&s->adsr, 0.05f, 0.2f, 0.5f, 0.5f);
	gwave_init(&s->gw, 0.5f, 0.5f, 1.f, midi_to_frequency(69), 0.f);

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
