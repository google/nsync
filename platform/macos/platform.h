#ifndef NSYNC_PLATFORM_MACOS_PLATFORM_H_
#define NSYNC_PLATFORM_MACOS_PLATFORM_H_

#define _DARWIN_C_SOURCE

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <inttypes.h>
#include <limits.h>
#include <sys/time.h>

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>

#define CLOCK_REALTIME 0
typedef int clockid_t;
int clock_gettime (clockid_t clk_id, struct timespec *tp);
#define TIMER_ABSTIME 1

#endif /*NSYNC_PLATFORM_MACOS_PLATFORM_H_*/
