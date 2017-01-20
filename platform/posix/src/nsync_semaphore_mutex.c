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

struct mutex_cond {
	pthread_mutex_t mu;
	pthread_cond_t cv;
	uint32_t i;
};

static nsync_semaphore *sem_big_enough_for_mutex_cond =
	(nsync_semaphore *) (1 /
	(sizeof (struct mutex_cond) <= sizeof (*sem_big_enough_for_mutex_cond)));

/* Initialize *s; the initial value is 0. */
void nsync_mu_semaphore_init (nsync_semaphore *s) {
	struct mutex_cond *mc = (struct mutex_cond *) s;
	ASSERT (pthread_mutex_init (&mc->mu, NULL) == 0);
	ASSERT (pthread_cond_init (&mc->cv, NULL) == 0);
	mc->i = 0;
}

/* Wait until the count of *s exceeds 0, and decrement it. */
void nsync_mu_semaphore_p (nsync_semaphore *s) {
	struct mutex_cond *mc = (struct mutex_cond *) s;
	ASSERT (pthread_mutex_lock (&mc->mu) == 0);
	while (mc->i == 0) {
		ASSERT (pthread_cond_wait (&mc->cv, &mc->mu) == 0);
	}
	mc->i = 0;
	ASSERT (pthread_mutex_unlock (&mc->mu) == 0);
}

/* Wait until one of:
   the count of *s is non-zero, in which case decrement *s and return 0;
   or abs_deadline expires, in which case return ETIMEDOUT. */
int nsync_mu_semaphore_p_with_deadline (nsync_semaphore *s,
					nsync_time abs_deadline) {
	struct mutex_cond *mc = (struct mutex_cond *)s;
	int res = 0;
	ASSERT (pthread_mutex_lock (&mc->mu) == 0);
	if (nsync_time_cmp (abs_deadline, nsync_time_no_deadline) == 0) {
		while (mc->i == 0) {
			ASSERT (pthread_cond_wait (&mc->cv, &mc->mu) == 0);
		}
		mc->i = 0;
	} else {
		struct timespec ts_deadline;
		memset (&ts_deadline, 0, sizeof (ts_deadline));
		ts_deadline.tv_sec = NSYNC_TIME_SEC (abs_deadline);
		ts_deadline.tv_nsec = NSYNC_TIME_NSEC (abs_deadline);
		while (mc->i == 0 &&
		       ((res = pthread_cond_timedwait (&mc->cv, &mc->mu, &ts_deadline)) == 0 ||
			(res == ETIMEDOUT && /* Various systems wake up too early. */
			 nsync_time_cmp (abs_deadline, nsync_time_now ()) > 0))) {
		}
		ASSERT (res == 0 || res == ETIMEDOUT);
		if (mc->i != 0) {
			res = 0;
			mc->i = 0;
		}
	}
	ASSERT (pthread_mutex_unlock (&mc->mu) == 0);
	return (res);
}

/* Ensure that the count of *s is at least 1. */
void nsync_mu_semaphore_v (nsync_semaphore *s) {
	struct mutex_cond *mc = (struct mutex_cond *) s;
	ASSERT (pthread_mutex_lock (&mc->mu) == 0);
	mc->i = 1;
	ASSERT (pthread_cond_broadcast (&mc->cv) == 0);
	ASSERT (pthread_mutex_unlock (&mc->mu) == 0);
}

NSYNC_CPP_END_
