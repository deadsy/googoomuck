//-----------------------------------------------------------------------------
/*

GGM Voice Functions

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

// lookup the voice being used for this channel and note.
struct voice *voice_lookup(struct ggm *s, uint8_t channel, uint8_t note) {
	for (unsigned int i = 0; i < NUM_VOICES; i++) {
		struct voice *ptr = &s->voices[i];
		if (ptr->note == note && ptr->channel == channel) {
			return ptr;
		}
	}
	return NULL;
}

struct voice *voice_alloc(struct ggm *s, uint8_t channel, uint8_t note) {
	// TODO
	return NULL;
}

//-----------------------------------------------------------------------------

// process a note off request
void voice_note_off(struct voice *v, uint8_t vel) {
}

// process a note on request
void voice_note_on(struct voice *v, uint8_t vel) {
}

//-----------------------------------------------------------------------------
