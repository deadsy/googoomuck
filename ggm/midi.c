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

// map a 0..127 midi control value from a..b
float midi_map(uint8_t val, float a, float b) {
	return a + ((b - a) / 127.f) * (float)(val & 0x7f);
}

// map a pitch bend value onto a note offset
float midi_pitch_bend(uint16_t val) {
	// 0..8192..16383 maps to -/+ 2 semitones
	return (float)(val - 8192) * (2.f / 8192.f);
}

// midi note to frequency conversion
// Note: treat the note as a float for pitch bending, tuning, etc.
float midi_to_frequency(float note) {
	return 440.f * pow2((note - 69.f) * (1.f / 12.f));
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
		v->patch->ops->note_off(v, vel);
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
		v->patch->ops->note_on(v, vel);
	}
}

// process a midi control change
static void midi_control_change(struct midi_rx *midi) {
	uint8_t chan = midi->status & 0xf;
	uint8_t ctrl = midi->arg0;
	uint8_t val = midi->arg1;
	if (ctrl >= 120) {
		// reserved controller number
		DBG("reserved control change ctrl %d val %d\r\n", ctrl, val);
		return;
	}
	//DBG("control change ch %d ctrl %d val %d\r\n", chan, ctrl, val);
	struct patch *p = &midi->ggm->patches[chan];
	if (p->ops) {
		p->ops->control_change(p, ctrl, val);
	}
}

// process a midi pitch wheel change
static void midi_pitch_wheel(struct midi_rx *midi) {
	uint8_t chan = midi->status & 0xf;
	uint16_t val = (midi->arg1 << 7) | midi->arg0;
	//DBG("pitch wheel ch %d val %d\r\n", chan, val);
	struct patch *p = &midi->ggm->patches[chan];
	if (p->ops) {
		p->ops->pitch_wheel(p, val);
	}
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
