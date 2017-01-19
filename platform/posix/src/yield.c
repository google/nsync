#include "headers.h"

NSYNC_CPP_START_

void nsync_yield_ (void) {
	sched_yield ();
}

NSYNC_CPP_END_
