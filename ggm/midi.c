//-----------------------------------------------------------------------------
/*

MIDI Functions

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

//-----------------------------------------------------------------------------

#define MIDI_NOTE_ON 9
#define MIDI_NOTE_OFF 8

// encode a midi message in event type bits 0..24
#define MIDI_MSG(t, c, x, y) \
  (((t) & 15) << 20 /*type*/) | \
  (((c) & 15) << 16 /*channel*/) | \
  (((x) & 127) << 8 /*x-parameter*/) | \
  ((y) & 127 /*y-parameter*/)

//-----------------------------------------------------------------------------

// midi note to frequency conversion
static const uint32_t m2f_table[128] = {
	0x4102d013U, 0x410a9760U, 0x4112d517U, 0x411b9041U,
	0x4124d054U, 0x412e9d37U, 0x4138ff49U, 0x4143ff6aU,
	0x414fa700U, 0x415c0000U, 0x416914f6U, 0x4176f110U,
	0x4182d013U, 0x418a9760U, 0x4192d517U, 0x419b9041U,
	0x41a4d054U, 0x41ae9d37U, 0x41b8ff49U, 0x41c3ff6aU,
	0x41cfa700U, 0x41dc0000U, 0x41e914f6U, 0x41f6f110U,
	0x4202d013U, 0x420a9760U, 0x4212d517U, 0x421b9041U,
	0x4224d054U, 0x422e9d37U, 0x4238ff49U, 0x4243ff6aU,
	0x424fa700U, 0x425c0000U, 0x426914f6U, 0x4276f110U,
	0x4282d013U, 0x428a9760U, 0x4292d517U, 0x429b9041U,
	0x42a4d054U, 0x42ae9d37U, 0x42b8ff49U, 0x42c3ff6aU,
	0x42cfa700U, 0x42dc0000U, 0x42e914f6U, 0x42f6f110U,
	0x4302d013U, 0x430a9760U, 0x4312d517U, 0x431b9041U,
	0x4324d054U, 0x432e9d37U, 0x4338ff49U, 0x4343ff6aU,
	0x434fa700U, 0x435c0000U, 0x436914f6U, 0x4376f110U,
	0x4382d013U, 0x438a9760U, 0x4392d517U, 0x439b9041U,
	0x43a4d054U, 0x43ae9d37U, 0x43b8ff49U, 0x43c3ff6aU,
	0x43cfa700U, 0x43dc0000U, 0x43e914f6U, 0x43f6f110U,
	0x4402d013U, 0x440a9760U, 0x4412d517U, 0x441b9041U,
	0x4424d054U, 0x442e9d37U, 0x4438ff49U, 0x4443ff6aU,
	0x444fa700U, 0x445c0000U, 0x446914f6U, 0x4476f110U,
	0x4482d013U, 0x448a9760U, 0x4492d517U, 0x449b9041U,
	0x44a4d054U, 0x44ae9d37U, 0x44b8ff49U, 0x44c3ff6aU,
	0x44cfa700U, 0x44dc0000U, 0x44e914f6U, 0x44f6f110U,
	0x4502d013U, 0x450a9760U, 0x4512d517U, 0x451b9041U,
	0x4524d054U, 0x452e9d37U, 0x4538ff49U, 0x4543ff6aU,
	0x454fa700U, 0x455c0000U, 0x456914f6U, 0x4576f110U,
	0x4582d013U, 0x458a9760U, 0x4592d517U, 0x459b9041U,
	0x45a4d054U, 0x45ae9d37U, 0x45b8ff49U, 0x45c3ff6aU,
	0x45cfa700U, 0x45dc0000U, 0x45e914f6U, 0x45f6f110U,
	0x4602d013U, 0x460a9760U, 0x4612d517U, 0x461b9041U,
	0x4624d054U, 0x462e9d37U, 0x4638ff49U, 0x4643ff6aU,
};

// return the frequency of the midi note
float midi_to_frequency(uint8_t note) {
	return *(float *)&m2f_table[note & 0x7f];
}

//-----------------------------------------------------------------------------

// generate a note on event for a midi channel
void midi_note_on(struct midi_drv *midi, uint8_t note, uint8_t velocity) {
	event_wr(EVENT_TYPE_MIDI | MIDI_MSG(MIDI_NOTE_ON, midi->ch, note, velocity), NULL);
}

// generate a note off event for a midi channel
void midi_note_off(struct midi_drv *midi, uint8_t note, uint8_t velocity) {
	event_wr(EVENT_TYPE_MIDI | MIDI_MSG(MIDI_NOTE_OFF, midi->ch, note, velocity), NULL);
}

//-----------------------------------------------------------------------------

enum {
	MIDI_STATE_COMMAND,
	MIDI_STATE_NOTE,
	MIDI_STATE_VELOCITY,
};

static void midi_rx(struct midi_drv *midi, uint8_t c) {
	switch (midi->state) {
	case MIDI_STATE_COMMAND:{
			uint8_t cmd = c & 0xf0;
			if (cmd == MIDI_NOTE_ON || cmd == MIDI_NOTE_OFF) {
				midi->cmd = cmd;
				midi->state = MIDI_STATE_NOTE;
			}
			break;
		}
	case MIDI_STATE_NOTE:{
			if ((c & 0x80) == 0) {
				midi->note = c;
				midi->state = MIDI_STATE_VELOCITY;
			} else {
				// not a valid note
				midi->state = MIDI_STATE_COMMAND;
			}
			break;
		}
	case MIDI_STATE_VELOCITY:{
			if ((c & 0x80) == 0) {
				event_wr(EVENT_TYPE_MIDI | MIDI_MSG(midi->cmd, midi->ch, midi->note, c), NULL);
			}
			midi->state = MIDI_STATE_COMMAND;
			break;
		}
	default:{
			// ?
			midi->state = MIDI_STATE_COMMAND;
			break;
		}
	}
}

// serial isr rx callback for midi
void midi_rx_isr(struct usart_drv *usart, uint8_t c) {
	midi_rx((struct midi_drv *)usart->priv, c);
}

//-----------------------------------------------------------------------------
