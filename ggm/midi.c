//-----------------------------------------------------------------------------
/*

MIDI Functions

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ggm.h"

#define DEBUG
#include "logging.h"

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
// MIDI Messages

// Channel Messages
#define MIDI_STATUS_NOTE_OFF (8U << 4)
#define MIDI_STATUS_NOTE_ON (9U << 4)
#define MIDI_STATUS_POLYPHONIC_AFTERTOUCH (10U << 4)
#define MIDI_STATUS_CONTROL_CHANGE (11U << 4)
#define MIDI_STATUS_PROGRAM_CHANGE (12U << 4)
#define MIDI_STATUS_CHANNEL_AFTERTOUCH (13U << 4)
#define MIDI_STATUS_PITCH_WHEEL (14U << 4)

// System Common Messages
#define MIDI_STATUS_SYSEX_START 0xF0
#define MIDI_STATUS_QUARTER_FRAME 0xF1
#define MIDI_STATUS_SONG_POINTER 0xF2
#define MIDI_STATUS_SONG_SELECT 0xF3
#define MIDI_STATUS_TUNE_REQUEST 0xF6
#define MIDI_STATUS_SYSEX_END 0xF7

// System Realtime Messages
#define MIDI_STATUS_TIMING_CLOCK 0xF8
#define MIDI_STATUS_START 0xFA
#define MIDI_STATUS_CONTINUE 0xFB
#define MIDI_STATUS_STOP 0xFC
#define MIDI_STATUS_ACTIVE_SENSING 0xFE
#define MIDI_STATUS_RESET 0xFF

// delimiters
#define MIDI_STATUS_COMMON 0xf0
#define MIDI_STATUS_REALTIME 0xf8

//-----------------------------------------------------------------------------
// channel events

// process a midi note off event
static void midi_note_off(struct midi_rx *midi) {
	uint8_t chan = midi->status & 0xf;
	uint8_t note = midi->arg0;
	uint8_t vel = midi->arg1;
	//DBG("note off ch %d note %d vel %d\r\n", chan, note, vel);
	struct voice *v = voice_lookup(midi->ggm, chan, note);
	if (v) {
		v->patch->note_off(v, vel);
	}
}

// process a midi note on event
static void midi_note_on(struct midi_rx *midi) {
	uint8_t chan = midi->status & 0xf;
	uint8_t note = midi->arg0;
	uint8_t vel = midi->arg1;
	if (vel == 0) {
		// velocity 0 == note off
		midi_note_off(midi);
		return;
	}
	//DBG("note on ch %d note %d vel %d\r\n", chan, note, vel);
	struct voice *v = voice_lookup(midi->ggm, chan, note);
	if (!v) {
		v = voice_alloc(midi->ggm, chan, note);
	}
	if (v) {
		v->patch->note_on(v, vel);
	}
}

// process a midi control change
static void midi_control_change(struct midi_rx *midi) {
	if (midi->arg0 >= 120) {
		// reserved controller number
		DBG("reserved control change ctrl %d val %d\r\n", midi->arg0, midi->arg1);
		return;
	}
	uint8_t ch = midi->status & 0xf;
	DBG("control change ch %d ctrl %d val %d\r\n", ch, midi->arg0, midi->arg1);
}

// process a midi pitch wheel change
static void midi_pitch_wheel(struct midi_rx *midi) {
	uint16_t val = (midi->arg1 << 7) | midi->arg0;
	uint8_t ch = midi->status & 0xf;
	DBG("pitch wheel ch %d val %d\r\n", ch, val);
}

// process a midi polyphonic aftertouch event
static void midi_polyphonic_aftertouch(struct midi_rx *midi) {
	uint8_t ch = midi->status & 0xf;
	DBG("polyphonic aftertouch ch %d key %d val %d\r\n", ch, midi->arg0, midi->arg1);
}

// process a midi program change
static void midi_program_change(struct midi_rx *midi) {
	uint8_t ch = midi->status & 0xf;
	DBG("program change ch %d val %d\r\n", ch, midi->arg0);
}

// process a midi channel aftertouch
static void midi_channel_aftertouch(struct midi_rx *midi) {
	uint8_t ch = midi->status & 0xf;
	DBG("channel aftertouch ch %d val %d\r\n", ch, midi->arg0);
}

//-----------------------------------------------------------------------------
// common events

static void midi_quarter_frame(struct midi_rx *midi) {
	DBG("quarter frame\r\n");
}

static void midi_song_pointer(struct midi_rx *midi) {
	DBG("song pointer\r\n");
}

static void midi_song_select(struct midi_rx *midi) {
	DBG("song select\r\n");
}

//-----------------------------------------------------------------------------
// sysex events

static void midi_sysex_start(struct midi_rx *midi) {
	DBG("sysex start\r\n");
}

// receive a sysex byte
static void midi_rx_sysex(struct midi_rx *midi, uint8_t x) {
	DBG("midi sysex %02x\r\n", x);
}

static void midi_sysex_end(struct midi_rx *midi) {
	DBG("sysex end\r\n");
}

//-----------------------------------------------------------------------------

// midi rx states
enum {
	MIDI_RX_NULL,		// ignore data
	MIDI_RX_1OF1,		// get 1st byte of 1
	MIDI_RX_1OF2,		// get 1st byte of 2
	MIDI_RX_2OF2,		// get 2nd byte of 2
	MIDI_RX_SYSEX,		// get system exclusive bytes
};

// Receive a buffer of midi bytes
static void midi_rxbuf(struct midi_rx *midi, uint8_t * buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		uint8_t c = buf[i];
		if (c & 0x80) {
			// status byte
			// any non-realtime status byte will end the sysex mode
			if (midi->state == MIDI_RX_SYSEX && c < MIDI_STATUS_REALTIME) {
				midi_sysex_end(midi);
				midi->state = MIDI_RX_NULL;
			}
			if (c < MIDI_STATUS_COMMON) {
				// channel message
				midi->status = c;
				switch (c & 0xf0) {
				case MIDI_STATUS_NOTE_OFF:
					midi->func = midi_note_off;
					midi->state = MIDI_RX_1OF2;
					break;
				case MIDI_STATUS_NOTE_ON:
					midi->func = midi_note_on;
					midi->state = MIDI_RX_1OF2;
					break;
				case MIDI_STATUS_POLYPHONIC_AFTERTOUCH:
					midi->func = midi_polyphonic_aftertouch;
					midi->state = MIDI_RX_1OF2;
					break;
				case MIDI_STATUS_CONTROL_CHANGE:
					midi->func = midi_control_change;
					midi->state = MIDI_RX_1OF2;
					break;
				case MIDI_STATUS_PROGRAM_CHANGE:
					midi->func = midi_program_change;
					midi->state = MIDI_RX_1OF1;
					break;
				case MIDI_STATUS_CHANNEL_AFTERTOUCH:
					midi->func = midi_channel_aftertouch;
					midi->state = MIDI_RX_1OF1;
					break;
				case MIDI_STATUS_PITCH_WHEEL:
					midi->func = midi_pitch_wheel;
					midi->state = MIDI_RX_1OF2;
					break;
				default:
					DBG("unhandled channel msg %02x\r\n", c);
					break;
				}
			} else if (c < MIDI_STATUS_REALTIME) {
				// system common message
				midi->status = 0;	// clear the running status
				switch (c) {
				case MIDI_STATUS_SYSEX_START:
					midi_sysex_start(midi);
					midi->state = MIDI_RX_SYSEX;
					break;
				case MIDI_STATUS_QUARTER_FRAME:
					midi->func = midi_quarter_frame;
					midi->state = MIDI_RX_1OF1;
					break;
				case MIDI_STATUS_SONG_POINTER:
					midi->func = midi_song_pointer;
					midi->state = MIDI_RX_1OF2;
					break;
				case MIDI_STATUS_SONG_SELECT:
					midi->func = midi_song_select;
					midi->state = MIDI_RX_1OF1;
					break;
				case MIDI_STATUS_TUNE_REQUEST:
					// no-op, we're a digital synth
					break;
				case MIDI_STATUS_SYSEX_END:
					// do nothing - we've already taken care of it.
					break;
				default:
					DBG("unhandled system commmon msg %02x\r\n", c);
					break;
				}
			} else {
				// system real time message
				switch (c) {
				case MIDI_STATUS_TIMING_CLOCK:
				case MIDI_STATUS_START:
				case MIDI_STATUS_CONTINUE:
				case MIDI_STATUS_STOP:
				case MIDI_STATUS_ACTIVE_SENSING:
				case MIDI_STATUS_RESET:
				default:
					DBG("unhandled system realtime msg %02x\r\n", c);
					break;
				}
			}
		} else {
			// data byte
			switch (midi->state) {
			case MIDI_RX_NULL:
				// ignore the data byte
				DBG("ignored data byte %02x\r\n", c);
				break;
			case MIDI_RX_1OF1:
				midi->arg0 = c;
				midi->func(midi);
				midi->state = (midi->status) ? MIDI_RX_1OF1 : MIDI_RX_NULL;
				break;
			case MIDI_RX_1OF2:
				midi->arg0 = c;
				midi->state = MIDI_RX_2OF2;
				break;
			case MIDI_RX_2OF2:
				midi->arg1 = c;
				midi->func(midi);
				midi->state = (midi->status) ? MIDI_RX_1OF2 : MIDI_RX_NULL;
				break;
			case MIDI_RX_SYSEX:
				midi_rx_sysex(midi, c);
				break;
			default:
				DBG("bug! unknown rx state %d\r\n", midi->state);
				midi->state = MIDI_RX_NULL;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------

// Receive midi messages from a serial port.
void midi_rx_serial(struct midi_rx *midi, struct usart_drv *serial) {
	// Use a buffer size large enough to get all serial bytes in a single read.
	// At the standard MIDI baud rate that's about 3 bytes/ms.
	uint8_t buf[16];
	size_t n;
	do {
		// read a buffer from the serial port
		n = usart_rxbuf(serial, buf, sizeof(buf));
		if (n != 0) {
			// write the buffer to the midi receiver
			midi_rxbuf(midi, buf, n);
		}
	} while (n == sizeof(buf));
}

//-----------------------------------------------------------------------------
