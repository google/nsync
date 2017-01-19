#include "headers.h"

NSYNC_CPP_START_

int nsync_clock_gettime (clockid_t clk_id UNUSED, struct timespec *tp) {
	struct timeb tb;
	ftime (&tb);
	tp->tv_sec = tb.time;
	tp->tv_nsec = tb.millitm * 1000 * 1000;
	return (0);
}

NSYNC_CPP_END_
