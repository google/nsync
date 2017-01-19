#ifndef NSYNC_INTERNAL_TIME_INTERNAL_H_
#define NSYNC_INTERNAL_TIME_INTERNAL_H_

#include "nsync_cpp.h"
#include "platform.h"
#include "nsync_time.h"

NSYNC_CPP_START_

extern nsync_time nsync_time_no_deadline; /* A deadline infinitely far in the future. */
extern nsync_time nsync_time_zero;  /* The zero delay, or an expired deadline. */

nsync_time nsync_time_now (void); /* Return the current time. */

/* Sleep for the specified delay.  Returns the unslept time
   which may be non-zero if the call was interrupted. */
nsync_time nsync_time_sleep (nsync_time delay);

/* Return a+b */
nsync_time nsync_time_add (nsync_time a, nsync_time b);

/* Return a-b */
nsync_time nsync_time_sub (nsync_time a, nsync_time b);

/*  Return +ve, 0, or -ve according to whether a>b, a==b, or a<b. */
int nsync_time_cmp (nsync_time a, nsync_time b);

/* Return the specified number of milliseconds as a time. */
nsync_time nsync_time_ms (unsigned ms);

/* Return the specified number of microseconds as a time. */
nsync_time nsync_time_us (unsigned us);

/* Return an nsync_time constructed from second and nanosecond components */
nsync_time nsync_time_s_ns (time_t s, uint32_t ns);

NSYNC_CPP_END_

#endif /*NSYNC_INTERNAL_TIME_INTERNAL_H_*/
