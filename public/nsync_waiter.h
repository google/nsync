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

#ifndef NSYNC_PUBLIC_NSYNC_WAITER_H_
#define NSYNC_PUBLIC_NSYNC_WAITER_H_

#include <inttypes.h>
#include <time.h>
#include "nsync_cpp.h"
#include "nsync_atomic.h"
#include "nsync_time.h"

NSYNC_CPP_START_

/* Implementations of "struct nsync_waitable_s" use this to allow waiting with nsync_wait_n(). */
struct nsync_semaphore_s_;
struct nsync_waiter_s {
	uint32_t tag;      /* used for debugging */
	struct nsync_semaphore_s_ *sem;  /* *sem will be Ved when waiter is woken */
	void *q[3];        /* really a nsync_dll_element_s_; used to link children of parent */
	nsync_atomic_uint32_ waiting;  /* non-zero <=> the waiter is waiting */
	uint32_t flags;    /* see below */
};
#define NSYNC_WAITER_FLAG_MUCV 0x1 /* set if waiter is embedded in Mu/CV's internal structures */

/* A "struct nsync_waitable_s" implementation must implement these functions. */
struct nsync_waitable_funcs_s {
	/* Return the time when *v will be ready (max time if
	   unknown), or 0 if it is already ready.  The parameter nw may be
	   passed as NULL, in which case the result should indicate whether the
	   thread would block if it were to wait on *v. */
	nsync_time (*ready_time) (void *v, struct nsync_waiter_s *nw);

	/* If *v is ready, return zero; otherwise enqueue *nw on *v and return
	   when *v will be ready (max time if unknown).  */
	nsync_time (*enqueue) (void *v, struct nsync_waiter_s *nw);

	/* If nw has been dequeued, return zero; otherwise dequeue *nw from *v and
	   return when *v will be ready (max time if unknown).  */
	nsync_time (*dequeue) (void *v, struct nsync_waiter_s *nw);
};

/* Clients can wait on a "struct nsync_waitable_s". */
struct nsync_waitable_s {
	void *v;
	const struct nsync_waitable_funcs_s *funcs;
};

/* The "struct nsync_waitable_s" for nsync_note and nsync_counter. */
extern const struct nsync_waitable_funcs_s nsync_note_waitable_funcs;
extern const struct nsync_waitable_funcs_s nsync_counter_waitable_funcs;
extern const struct nsync_waitable_funcs_s nsync_cv_waitable_funcs;

/* Wait until at least one of *waitable[0,..,count-1] is ready been notified or abs_deadline
   is reached.  Return the index of the notified element of waitable[], or count
   if no such element exists.   If mu!=NULL, (*unlock)(mu) is called after the thread is queued
   on the various waiters, and (*lock)(mu) is called before return. */
int nsync_wait_n (void *mu, void (*lock) (void *), void (*unlock) (void *),
		  nsync_time abs_deadline, int count,
		  struct nsync_waitable_s *waitable[]);

NSYNC_CPP_END_

#endif /*NSYNC_PUBLIC_NSYNC_WAITER_H_*/
