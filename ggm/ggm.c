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
// voice operations

// lookup the voice being used for this channel and note.
struct voice *voice_lookup(struct ggm *s, uint8_t channel, uint8_t note) {
	for (unsigned int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		if (v->note == note && v->channel == channel) {
			return v;
		}
	}
	return NULL;
}

// allocate a new voice, possibly reusing a current active voice.
struct voice *voice_alloc(struct ggm *s, uint8_t channel, uint8_t note) {
	// validate the channel
	if (channel >= NUM_CHANNELS || s->patches[channel].ops == NULL) {
		DBG("no patch defined for channel %d\r\n", channel);
		return NULL;
	}
	// TODO: Currently doing simple round robin allocation.
	// More intelligent voice allocation to follow....
	struct voice *v = &s->voices[s->voice_idx];
	s->voice_idx += 1;
	if (s->voice_idx == NUM_VOICES) {
		s->voice_idx = 0;
	}
	// stop an existing patch on this voice
	if (v->patch) {
		v->patch->ops->stop(v);
	}
	// setup the new voice
	v->note = note;
	v->channel = channel;
	v->patch = &s->patches[channel];
	v->patch->ops->start(v);

	return v;
}

//-----------------------------------------------------------------------------
// key events

// handle a key down event
static void key_dn_handler(struct ggm *s, struct event *e) {
	DBG("key down %d\r\n", EVENT_KEY(e->type));
	led_on(LED_BLUE);
}

// handle a key up event
static void key_up_handler(struct ggm *s, struct event *e) {
	DBG("key up %d\r\n", EVENT_KEY(e->type));
	led_off(LED_BLUE);
}

//-----------------------------------------------------------------------------
// midi events

// Currently unused.
// Serial MIDI events are handled as they are read from the buffer.
// MIDI events from USB might use this....

// handle a midi event
static void midi_handler(struct ggm *s, struct event *e) {
	DBG("midi %06x\r\n", EVENT_MIDI(e->type));
}

//-----------------------------------------------------------------------------
// audio request events

// handle an audio request event
static void audio_handler(struct ggm *s, struct event *e) {
	size_t n = EVENT_BLOCK_SIZE(e->type);
	int16_t *dst = e->ptr;
	float out[n];

	//DBG("audio %08x %08x\r\n", e->type, e->ptr);

	memset(out, 0, n * sizeof(float));
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		struct patch *p = v->patch;
		if (p && p->ops->active(v)) {
			float buf[n];
			p->ops->generate(v, buf, n);
			block_add(out, buf, n);
		}
	}

	// write the samples to the dma buffer
	audio_wr(dst, n, out, out);
	// record some realtime stats
	audio_stats(s->audio, dst);
}

//-----------------------------------------------------------------------------

// the main ggm event loop
int ggm_run(struct ggm *s) {
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
		midi_rx_serial(&s->midi_rx0, s->serial);
	}
	return 0;
}

//-----------------------------------------------------------------------------

// initialise the ggm state
int ggm_init(struct ggm *s, struct audio_drv *audio, struct usart_drv *serial) {
	int rc = 0;

	memset(s, 0, sizeof(struct ggm));
	s->audio = audio;
	s->serial = serial;

	// setup the midi receivers.
	s->midi_rx0.ggm = s;

	rc = event_init();
	if (rc != 0) {
		DBG("event_init failed %d\r\n", rc);
		goto exit;
	}
	// setup the patch operations
	s->patches[0].ops = &patch3;
	s->patches[1].ops = &patch2;
	s->patches[2].ops = &patch1;
	s->patches[3].ops = &patch0;
	s->patches[4].ops = &patch4;

	// setup the patch on each channel
	for (int i = 0; i < NUM_CHANNELS; i++) {
		struct patch *p = &s->patches[i];
		// call init for each patch
		if (p->ops) {
			p->ggm = s;
			p->ops->init(p);
		}
	}

	// setup the voices
	for (int i = 0; i < NUM_VOICES; i++) {
		struct voice *v = &s->voices[i];
		v->idx = i;
		v->channel = 255;
		v->note = 255;
	}

 exit:
	return rc;
}

//-----------------------------------------------------------------------------
