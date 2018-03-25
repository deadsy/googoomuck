//-----------------------------------------------------------------------------
/*

Note Sequencer

The sequencer clock is the rate blocks of audio samples are sent to the audio CODEC.
This is divided down to give the desired beats per minute.
Each beat is a quarter note. Each beat is divided into TICKS_PER_BEAT ticks.
Note durations are specified with a tick count.

*/
//-----------------------------------------------------------------------------

#include "ggm.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

#define TICKS_PER_BEAT (16)
#define SECS_PER_MIN (60.f)
#define SECS_PER_BLOCK (AUDIO_BLOCK_SIZE / AUDIO_FS)

//-----------------------------------------------------------------------------
// Note durations

#define DURATION1 64		// whole note
#define DURATION2 32		// half note
#define DURATION4 16		// quarter note
#define DURATION8 8		// 1/8 note
#define DURATION16 4		// 1/16 note
#define DURATION32 2		// 1/32 note
#define DURATION64 1		// 1/64 note

//-----------------------------------------------------------------------------
// states

// sequencer state
enum {
	S_STATE_STOP,		// 0
	S_STATE_RUN,
};

// operation state
enum {
	O_STATE_INIT,		// 0
	O_STATE_WAIT,
};

//-----------------------------------------------------------------------------

struct note_args {
	uint8_t op;
	uint8_t chan;
	uint8_t note;
	uint8_t vel;
	uint8_t dur;
};

struct rest_args {
	uint8_t op;
	uint8_t dur;
};

//-----------------------------------------------------------------------------

// process a sequencer note off event
static void seq_note_off(struct seq *s, struct note_args *args) {
	struct voice *v = voice_lookup(s->ggm, args->chan, args->note);
	if (v) {
		v->patch->ops->note_off(v, 0);
	}
}

// process a sequencer note on event
static void seq_note_on(struct seq *s, struct note_args *args) {
	struct voice *v = voice_lookup(s->ggm, args->chan, args->note);
	if (!v) {
		v = voice_alloc(s->ggm, args->chan, args->note);
	}
	if (v) {
		v->patch->ops->note_on(v, args->vel);
	}
}

//-----------------------------------------------------------------------------
// opcodes

// no operation, (op)
static int op_nop(struct seq *s, struct seq_sm *m) {
	return 1;
}

// return to beginning, (op)
static int op_loop(struct seq *s, struct seq_sm *m) {
	m->pc = -1;
	return 1;
}

// note on/off, (op, channel, note, velocity, duration)
static int op_note(struct seq *s, struct seq_sm *m) {
	struct note_args *args = (struct note_args *)&m->prog[m->pc];
	if (m->op_state == O_STATE_INIT) {
		// init
		m->duration = args->dur;
		m->op_state = O_STATE_WAIT;
		DBG("note on %d (%d)\r\n", args->note, s->ticks);
		seq_note_on(s, args);
	}
	m->duration -= 1;
	if (m->duration == 0) {
		// done
		m->op_state = O_STATE_INIT;
		DBG("note off (%d)\r\n", s->ticks);
		seq_note_off(s, args);
		return sizeof(struct note_args);
	}
	// waiting...
	return 0;
}

// rest (op, duration)
static int op_rest(struct seq *s, struct seq_sm *m) {
	struct rest_args *args = (struct rest_args *)&m->prog[m->pc];
	if (m->op_state == O_STATE_INIT) {
		// init
		m->duration = args->dur;
		m->op_state = O_STATE_WAIT;
	}
	m->duration -= 1;
	if (m->duration == 0) {
		// done
		m->op_state = O_STATE_INIT;
		return sizeof(struct rest_args);
	}
	// waiting...
	return 0;
}

enum {
	OP_NOP,			// no operation
	OP_LOOP,		// return to beginning
	OP_NOTE,		// note on/off
	OP_REST,		// rest
	OP_NUM			// must be last
};

static int (*op_table[OP_NUM]) (struct seq * s, struct seq_sm * m) = {
	op_nop,			// OP_NOP
	    op_loop,		// OP_LOOP
	    op_note,		// OP_NOTE
	    op_rest,		// OP_REST
};

static void ssm_tick(struct seq *s, struct seq_sm *m) {
	if (m->s_state == S_STATE_RUN) {
		int (*op) (struct seq * s, struct seq_sm * m) = op_table[m->prog[m->pc]];
		m->pc += op(s, m);
	}
}

//-----------------------------------------------------------------------------

static uint8_t metronome[] = {
	OP_NOTE, 1, 69, 100, 4,
	OP_REST, 12,
	OP_NOTE, 1, 60, 100, 4,
	OP_REST, 12,
	OP_NOTE, 1, 60, 100, 4,
	OP_REST, 12,
	OP_NOTE, 1, 60, 100, 4,
	OP_REST, 12,
	OP_LOOP,
};

//-----------------------------------------------------------------------------

void seq_exec(struct seq *s) {
	// The desired BPM will generally not correspond to an integral number
	// of audio blocks, so accumulate an error and tick when needed.
	// ie- Bresenham style.
	s->tick_error += SECS_PER_BLOCK;
	if (s->tick_error > s->secs_per_tick) {
		s->tick_error -= s->secs_per_tick;
		// tick...
		s->ticks++;
		ssm_tick(s, &s->m0);
	}
}

//-----------------------------------------------------------------------------

int seq_init(struct seq *s) {

	s->beats_per_min = 120.f;
	s->secs_per_tick = SECS_PER_MIN / (s->beats_per_min * (float)TICKS_PER_BEAT);
	DBG("secs_per_tick %08x\r\n", *(uint32_t *) & s->secs_per_tick);

	float secs_per_block = SECS_PER_BLOCK;
	DBG("secs_per_block %08x\r\n", *(uint32_t *) & secs_per_block);

	s->m0.prog = metronome;
	s->m0.s_state = S_STATE_RUN;

	return 0;
}

//-----------------------------------------------------------------------------
