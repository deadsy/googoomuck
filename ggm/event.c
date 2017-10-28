//-----------------------------------------------------------------------------
/*

Events

The main loop of GGM is an event processor. Events are generated asynchronously
by the world. E.g a request to fill the audio buffer with samples, a midi event,
a key down event, etc. The event loop branches to a function to handle the event.
There is no priority, we just service events as they arrive.

The event queue is a circular buffer. Producers (ISRs or self generated events)
write to it. Consumers (the main event loop) read from it. The event queue should
never be full. If it is we have probably blundered.

The events are a uint32_t and a pointer. Based on the event type the pointer
may reference some other data structure/buffer. Or it might be some additional
meta data, or it might be empty. This code doesn't care, that's all up to the
user of the events.

*/
//-----------------------------------------------------------------------------

#include <string.h>

#include "ggm.h"

//-----------------------------------------------------------------------------

#define NUM_EVENTS 16		// must be a power of 2

// circular buffer for events
struct event_queue {
	struct event queue[NUM_EVENTS];
	size_t rd;
	size_t wr;
};

static struct event_queue eq;

//-----------------------------------------------------------------------------

// read an event from the event queue
int event_rd(struct event *e) {
	struct event *x;
	uint32_t saved;
	int rc = 0;
	// mask interrupts
	saved = disable_irq();
	// do we have events?
	if (eq.rd == eq.wr) {
		// no events
		rc = -1;
		goto exit;
	}
	// copy the event data
	x = &eq.queue[eq.rd];
	e->type = x->type;
	e->ptr = x->ptr;
	// advance the read index
	eq.rd = (eq.rd + 1) & (NUM_EVENTS - 1);
 exit:
	// restore interrupts and return
	restore_irq(saved);
	return rc;
}

// write an event to the event queue
int event_wr(uint32_t type, void *ptr) {
	struct event *x;
	uint32_t saved;
	size_t wr;
	int rc = 0;
	// mask interrupts
	saved = disable_irq();
	wr = (eq.wr + 1) & (NUM_EVENTS - 1);
	if (wr == eq.rd) {
		// the queue is full
		rc = -1;
		goto exit;
	}
	// copy the event data
	x = &eq.queue[eq.wr];
	x->type = type;
	x->ptr = ptr;
	// advance the write index
	eq.wr = wr;
 exit:
	// restore interrupts and return
	restore_irq(saved);
	return rc;
}

//-----------------------------------------------------------------------------

// initialise event processing
int event_init(void) {
	memset(&eq, 0, sizeof(struct event_queue));
	return 0;
}

//-----------------------------------------------------------------------------
