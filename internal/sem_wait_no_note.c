#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync.h"
#include "dll.h"
#include "sem.h"
#include "common.h"
#include "atomic.h"

NSYNC_CPP_START_

/* Wait until one of:
     w->sem is non-zero----decrement it and return 0.
     abs_deadline expires---return ETIMEDOUT.
     Ignores cancel_note. */
int nsync_sem_wait_with_cancel_ (waiter *w, nsync_time abs_deadline, nsync_note *cancel_note) {
	return (nsync_mu_semaphore_p_with_deadline (&w->sem, abs_deadline));
}

NSYNC_CPP_END_
