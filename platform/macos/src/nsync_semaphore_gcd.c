/* Copyright 2019 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

#include "headers.h"
#pragma GCC diagnostic ignored "-Wlong-long"  /* to avoid error on DISPATCH_TIME_FOREVER */
#include <dispatch/dispatch.h>

NSYNC_CPP_START_

static nsync_semaphore *sem_big_enough_for_dispatch_semaphore_t = (void *) (1 /
	(sizeof (dispatch_semaphore_t) <= sizeof (*sem_big_enough_for_dispatch_semaphore_t)));

/* Initialize *s; the initial value is 0.  */
void nsync_mu_semaphore_init (nsync_semaphore *s) {
	*(dispatch_semaphore_t *)s = dispatch_semaphore_create (0);
}

/* Wait until the count of *s exceeds 0, and decrement it. */
void nsync_mu_semaphore_p (nsync_semaphore *s) {
	dispatch_semaphore_wait (*(dispatch_semaphore_t *)s, DISPATCH_TIME_FOREVER);
}

/* Wait until one of:
   the count of *s is non-zero, in which case decrement *s and return 0;
   or abs_deadline expires, in which case return ETIMEDOUT. */
int nsync_mu_semaphore_p_with_deadline (nsync_semaphore *s,
					nsync_time abs_deadline) {
	int result = 0;
	if (nsync_time_cmp (abs_deadline, nsync_time_no_deadline) == 0) {
		dispatch_semaphore_wait (*(dispatch_semaphore_t *)s, DISPATCH_TIME_FOREVER);
	} else {
		struct timespec ts;
		memset (&ts, 0, sizeof (ts));
		ts.tv_sec = NSYNC_TIME_SEC (abs_deadline);
		ts.tv_nsec = NSYNC_TIME_NSEC (abs_deadline);
		if (dispatch_semaphore_wait (*(dispatch_semaphore_t *)s,
					     dispatch_walltime (&abs_deadline, 0)) != 0) {
			result = ETIMEDOUT;
		}
	}
	return (result);
}

/* Ensure that the count of *s is at least 1. */
void nsync_mu_semaphore_v (nsync_semaphore *s) {
	dispatch_semaphore_signal (*(dispatch_semaphore_t *)s);
}

NSYNC_CPP_END_
