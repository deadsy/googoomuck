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
// MIDI Messages

// MIDI Channel Messages
#define MIDI_STATUS_NOTE_OFF (8U << 4)
#define MIDI_STATUS_NOTE_ON (9U << 4)
#define MIDI_STATUS_POLYPHONIC_AFTERTOUCH (10U << 4)
#define MIDI_STATUS_CONTROL_CHANGE (11U << 4)
#define MIDI_STATUS_PROGRAM_CHANGE (12U << 4)
#define MIDI_STATUS_CHANNEL_AFTERTOUCH (13U << 4)
#define MIDI_STATUS_PITCH_WHEEL (14U << 4)
#define MIDI_STATUS_SYSTEM (15U << 4)

// MIDI System Exclusive
#define MIDI_STATUS_SYSEX_START 0xF0
#define MIDI_STATUS_SYSEX_END 0xF7

// MIDI System Common
#define MIDI_STATUS_QUARTER_FRAME 0xF1
#define MIDI_STATUS_SONG_POINTER 0xF2
#define MIDI_STATUS_SONG_SELECT 0xF3
#define MIDI_STATUS_TUNE_REQUEST 0xF6

// MIDI System Realtime
#define MIDI_STATUS_TIMING_CLOCK 0xF8
#define MIDI_STATUS_START 0xFA
#define MIDI_STATUS_CONTINUE 0xFB
#define MIDI_STATUS_STOP 0xFC
#define MIDI_STATUS_ACTIVE_SENSING 0xFE
#define MIDI_STATUS_RESET 0xFF

//-----------------------------------------------------------------------------

#if 0
// encode a midi message in event type bits 0..24
#define MIDI_MSG(t, c, x, y) \
  (((t) & 15) << 20 /*type*/) | \
  (((c) & 15) << 16 /*channel*/) | \
  (((x) & 127) << 8 /*x-parameter*/) | \
  ((y) & 127 /*y-parameter*/)
#endif

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

static void midi_note_off(struct midi_drv *midi) {
	DBG("midi note off c %d n %d v %d\r\n", midi->channel, midi->arg0, midi->arg1);
}

static void midi_note_on(struct midi_drv *midi) {
	DBG("midi note on c %d n %d v %d\r\n", midi->channel, midi->arg0, midi->arg1);
}

static void midi_sysex_end(struct midi_drv *midi) {
	DBG("midi sysex end\r\n");
}

//-----------------------------------------------------------------------------

// midi rx states
enum {
	MIDI_RX_STATUS,		// get the status byte
	MIDI_RX_1OF1,		// get 1st byte of 1
	MIDI_RX_1OF2,		// get 1st byte of 2
	MIDI_RX_2OF2,		// get 2nd byte of 2
	MIDI_RX_SYSEX,		// get system exclusive bytes
};

// reset the midi rx state
static void midi_rx_reset(struct midi_drv *midi) {
	midi->state = MIDI_RX_STATUS;
	midi->func = NULL;
}

// receive a sysex byte
static void midi_rx_sysex(struct midi_drv *midi, uint8_t x) {
	DBG("midi sysex %02x\r\n", x);
}

static void midi_rx_status(struct midi_drv *midi, uint8_t status) {
	// get the channel
	midi->channel = (status & 0x0f) + 1;
	// process the status byte
	uint8_t chmsg = status & 0xf0;
	if (chmsg == MIDI_STATUS_NOTE_OFF) {
		midi->func = midi_note_off;
		midi->state = MIDI_RX_1OF2;
	} else if (chmsg == MIDI_STATUS_NOTE_ON) {
		midi->func = midi_note_on;
		midi->state = MIDI_RX_1OF2;
	} else if (chmsg == MIDI_STATUS_POLYPHONIC_AFTERTOUCH) {
		// TODO: midi->func =
		midi->state = MIDI_RX_1OF2;
	} else if (chmsg == MIDI_STATUS_CONTROL_CHANGE) {
		// TODO: midi->func =
		midi->state = MIDI_RX_1OF2;
	} else if (chmsg == MIDI_STATUS_PROGRAM_CHANGE) {
		// TODO: midi->func =
		midi->state = MIDI_RX_1OF1;
	} else if (chmsg == MIDI_STATUS_CHANNEL_AFTERTOUCH) {
		// TODO: midi->func =
		midi->state = MIDI_RX_1OF1;
	} else if (chmsg == MIDI_STATUS_PITCH_WHEEL) {
		// TODO: midi->func =
		midi->state = MIDI_RX_1OF2;
	} else if (chmsg == MIDI_STATUS_SYSTEM) {
		if (status == MIDI_STATUS_SYSEX_START) {
			midi->state = MIDI_RX_SYSEX;
		} else if (status == MIDI_STATUS_QUARTER_FRAME) {
			// TODO: midi->func =
			midi->state = MIDI_RX_1OF1;
		} else if (status == MIDI_STATUS_SONG_POINTER) {
			// TODO: midi->func =
			midi->state = MIDI_RX_1OF2;
		} else if (status == MIDI_STATUS_SONG_SELECT) {
			// TODO: midi->func =
			midi->state = MIDI_RX_1OF1;
		} else if (status == MIDI_STATUS_TUNE_REQUEST) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_TIMING_CLOCK) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_START) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_CONTINUE) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_STOP) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_ACTIVE_SENSING) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else if (status == MIDI_STATUS_RESET) {
			// TODO: 0 arguments
			midi_rx_reset(midi);
		} else {
			DBG("unknown system message %02x\r\n", status);
			midi_rx_reset(midi);
		}
	} else {
		DBG("unknown channel message %02x\r\n", status);
		midi_rx_reset(midi);
	}
}

// Receive a buffer of midi bytes
static void midi_rxbuf(struct midi_drv *midi, uint8_t * buf, size_t n) {
	for (size_t i = 0; i < n; i++) {
		uint8_t c = buf[i];
		if (midi->state == MIDI_RX_STATUS) {
			// process the satus byte
			midi_rx_status(midi, c);
		} else if (midi->state == MIDI_RX_1OF1) {
			// call the midi function (1 argument)
			if ((c & 0x80) == 0) {
				midi->arg0 = c;
				if (midi->func) {
					midi->func(midi);
				}
			} else {
				DBG("bad midi argument %02x\r\n", c);
			}
			midi_rx_reset(midi);
		} else if (midi->state == MIDI_RX_1OF2) {
			// get the 1st argument
			if ((c & 0x80) == 0) {
				midi->arg0 = c;
				midi->state = MIDI_RX_2OF2;
			} else {
				DBG("bad midi argument %02x\r\n", c);
				midi_rx_reset(midi);
			}
		} else if (midi->state == MIDI_RX_2OF2) {
			// call the midi function (2 arguments)
			if ((c & 0x80) == 0) {
				midi->arg1 = c;
				if (midi->func) {
					midi->func(midi);
				}
			} else {
				DBG("bad midi argument %02x\r\n", c);
			}
			midi_rx_reset(midi);
		} else if (midi->state == MIDI_RX_SYSEX) {
			// process sysex bytes
			if ((c & 0x80) == 0) {
				midi_rx_sysex(midi, c);
			} else {
				if (c == MIDI_STATUS_SYSEX_END) {
					midi_sysex_end(midi);
				} else {
					DBG("unknown sysex byte %02x\r\n", c);
				}
				midi_rx_reset(midi);
			}
		} else {
			DBG("unknown midi state %d\r\n", midi->state);
			midi_rx_reset(midi);
		}
	}
}

//-----------------------------------------------------------------------------

// Receive midi messages from a serial port.
void midi_rx_serial(struct midi_drv *midi, struct usart_drv *serial) {
	uint8_t buf[16];
	size_t n;
	do {
		// read a buffer from the serial port
		n = usart_rxbuf(serial, buf, sizeof(buf));
		// write the buffer to the midi receiver
		midi_rxbuf(midi, buf, n);
	} while (n == sizeof(buf));
}

//-----------------------------------------------------------------------------
