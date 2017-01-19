#ifndef NSYNC_PUBLIC_NSYNC_TIME_H_
#define NSYNC_PUBLIC_NSYNC_TIME_H_

#include "nsync_cpp.h"

/* This file is not to be included directly by the client.  It exists because
   different environments insist on different representations of time,
   sometimes because they allow their realtime clocks to go backwards (!).  */

#if NSYNC_USE_GPR_TIMESPEC
#include "grpc/support/time.h"
NSYNC_CPP_START_
typedef gpr_timespec nsync_time;
#define NSYNC_TIME_SEC(t) ((t).tv_sec)
#define NSYNC_TIME_NSEC(t) ((t).tv_nsec)
NSYNC_CPP_END_

#elif NSYNC_USE_DEBUG_TIME
/* Check that the library can be built with a different time struct.  */
#include <time.h>
NSYNC_CPP_START_
typedef struct {
	time_t seconds;
	unsigned nanoseconds;
} nsync_time;
#define NSYNC_TIME_SEC(t) ((t).seconds)
#define NSYNC_TIME_NSEC(t) ((t).nanoseconds)
NSYNC_CPP_END_

#else
/* Default is to use timespec. */
struct timespec;
NSYNC_CPP_START_
typedef struct timespec nsync_time;
#define NSYNC_TIME_SEC(t) ((t).tv_sec)
#define NSYNC_TIME_NSEC(t) ((t).tv_nsec)
NSYNC_CPP_END_

#endif

#endif /*NSYNC_PUBLIC_NSYNC_TIME_H_*/
