#include "headers.h"

NSYNC_CPP_START_

int nsync_nanosleep (const struct timespec *request, struct timespec *remain) {
	time_t x = request->tv_sec;
	while (x > 1000) {
		x -= 1000;
		Sleep (1000 * 1000);
	}
	Sleep (x * 1000 + (request->tv_nsec + 999999) / (1000 * 1000));
	if (remain != NULL) {
		memset (remain, 0, sizeof (*remain));
	}
	return (0);
}

NSYNC_CPP_END_
