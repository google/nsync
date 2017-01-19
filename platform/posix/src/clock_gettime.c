#include "headers.h"

NSYNC_CPP_START_

int clock_gettime(clockid_t clk_id UNUSED, struct timespec *tp) {
	struct timeval tv;
	int rc = gettimeofday(&tv, NULL);
	if (rc == 0) {
		tp->tv_sec = tv.tv_sec;
		tp->tv_nsec = tv.tv_usec * 1000;
	}
	return (rc);
}

NSYNC_CPP_END_
