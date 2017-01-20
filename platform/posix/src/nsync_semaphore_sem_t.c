/* Copyright 2016 Google Inc.

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

NSYNC_CPP_START_

#define ASSERT(x) do { if (!(x)) { *(volatile int *)0 = 0; } } while (0)

static nsync_semaphore *sem_big_enough_for_sem_t = (void *) (1 /
	(sizeof (sem_t) <= sizeof (*sem_big_enough_for_sem_t)));

/* Initialize *s; the initial value is 0.  */
void nsync_mu_semaphore_init (nsync_semaphore *s) {
	ASSERT (sem_init ((sem_t *)s, 0, 0) == 0);
}

/* Wait until the count of *s exceeds 0, and decrement it. */
void nsync_mu_semaphore_p (nsync_semaphore *s) {
	while (sem_wait ((sem_t *)s) != 0 && errno == EINTR) {
	}
}

/* Wait until one of:
   the count of *s is non-zero, in which case decrement *s and return 0;
   or abs_deadline expires, in which case return ETIMEDOUT. */
int nsync_mu_semaphore_p_with_deadline (nsync_semaphore *s,
					nsync_time abs_deadline) {
	int result = 0;
	if (nsync_time_cmp (abs_deadline, nsync_time_no_deadline) == 0) {
		while (sem_wait ((sem_t *)s) != 0 && errno == EINTR) {
		}
	} else {
		int rc;
		struct timespec ts;
		memset (&ts, 0, sizeof (ts));
		ts.tv_sec = NSYNC_TIME_SEC (abs_deadline);
		ts.tv_nsec = NSYNC_TIME_NSEC (abs_deadline);
		do {
			rc = sem_timedwait ((sem_t *)s, &ts);
		} while (rc != 0 &&
			 (errno == EINTR ||
		          (errno == ETIMEDOUT &&
			   nsync_time_cmp (abs_deadline, nsync_time_now ()) > 0)));
		if (rc != 0 && errno == ETIMEDOUT) {
			result = ETIMEDOUT;
		}
	}
	return (result);
}

/* Ensure that the count of *s is at least 1. */
void nsync_mu_semaphore_v (nsync_semaphore *s) {
	ASSERT (sem_post ((sem_t *)s) == 0);
}

NSYNC_CPP_END_
