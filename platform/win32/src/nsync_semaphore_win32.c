#include <Windows.h>
#include "nsync_cpp.h"
#include "nsync_time.h"
#include "time_internal.h"
#include "sem.h"

NSYNC_CPP_START_

/* Initialize *s; the initial value is 0. */
void nsync_mu_semaphore_init (nsync_semaphore *s) {
	HANDLE *h = (HANDLE *) &s->sem;
	*h = CreateSemaphore(NULL, 0, 1, NULL);
	if (*h == NULL) {
		abort ();
	}
}

/* Wait until the count of *s exceeds 0, and decrement it. */
void nsync_mu_semaphore_p (nsync_semaphore *s) {
	HANDLE *h = (HANDLE *) &s->sem;
	WaitForSingleObject(*h, INFINITE);
}

/* Wait until one of:
   the count of *s is non-zero, in which case decrement *s and return 0;
   or abs_deadline expires, in which case return ETIMEDOUT. */
int nsync_mu_semaphore_p_with_deadline (nsync_semaphore *s, nsync_time abs_deadline) {
	HANDLE *h = (HANDLE *) &s->sem;
	int result;

	if (nsync_time_cmp (abs_deadline, nsync_time_no_deadline) == 0) {
		result = WaitForSingleObject(*h, INFINITE);
	} else {
		nsync_time now;
		now = nsync_time_now ();
		do {
			if (nsync_time_cmp (abs_deadline, now) <= 0) {
				result = WaitForSingleObject (*h, 0);
			} else {
				nsync_time delay;
				delay = nsync_time_sub (abs_deadline, now);
				if (NSYNC_TIME_SEC (delay) > 1000*1000) {
					result = WaitForSingleObject (*h, 1000*1000);
				} else {
					result = WaitForSingleObject (*h,
						NSYNC_TIME_SEC (delay) * 1000 +
							(NSYNC_TIME_NSEC (delay) + 999999) / (1000 * 1000));
				}
			}
			if (result == WAIT_TIMEOUT) {
				now = nsync_time_now ();
			}
		} while (result == WAIT_TIMEOUT && /* Windows generates early wakeups. */
			 nsync_time_cmp (abs_deadline, now) > 0);
	}
	return (result == WAIT_TIMEOUT? ETIMEDOUT : 0);
}

/* Ensure that the count of *s is at least 1. */
void nsync_mu_semaphore_v (nsync_semaphore *s) {
	HANDLE *h = (HANDLE *) &s->sem;
	ReleaseSemaphore(*h, 1, NULL);
}

NSYNC_CPP_END_
