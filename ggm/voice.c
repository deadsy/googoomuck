//-----------------------------------------------------------------------------
/*

GGM Voice Functions

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

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
	if (channel >= NUM_CHANNELS || s->channel_to_patch[channel] == NULL) {
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
		v->patch->stop(v);
	}
	// setup the new voice
	v->note = note;
	v->channel = channel;
	v->patch = s->channel_to_patch[channel];
	v->patch->start(v);

	return v;
}

//-----------------------------------------------------------------------------
