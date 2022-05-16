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

/* This package provides a mutex nsync_mu and a Mesa-style condition variable nsync_cv. */

#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync.h"
#include "atomic.h"
#include "sem.h"
#include "dll.h"
#include "wait_internal.h"
#include "common.h"

NSYNC_CPP_START_

/* Implementation notes

   The implementations of nsync_mu and nsync_cv both use spinlocks to protect
   their waiter queues.  The spinlocks are implemented with atomic operations
   and a delay loop found below.  They could use pthread_mutex_t, but I wished
   to have an implementation independent of pthread mutexes and condition
   variables.

   nsync_mu and nsync_cv use the same type of doubly-linked list of waiters
   (see waiter.c).  This allows waiters to be transferred from the cv queue to
   the mu queue when a thread is logically woken from the cv but would
   immediately go to sleep on the mu.  See the wake_waiters() call.

   In mu, the "designated waker" is a thread that was waiting on mu, has been
   woken up, but as yet has neither acquired nor gone back to waiting.  The
   presence of such a thread is indicated by the MU_DESIG_WAKER bit in the mu
   word.  This bit allows the nsync_mu_unlock() code to avoid waking a second
   waiter when there's already one that will wake the next thread when the time
   comes.  This speeds things up when the lock is heavily contended, and the
   critical sections are small.

   The weasel words "with high probability" in the specification of
   nsync_mu_trylock() and nsync_mu_rtrylock() prevent clients from believing
   that they can determine with certainty whether another thread has given up a
   lock yet.  This, together with the requirement that a thread that acquired a
   mutex must release it (rather than it being released by another thread),
   prohibits clients from using mu as a sort of semaphore.  The intent is that
   it be used only for traditional mutual exclusion, and that clients that need
   a semaphore should use one.  This leaves room for certain future
   optimizations, and make it easier to apply detection of potential races via
   candidate lock-set algorithms, should that ever be desired.

   The nsync_mu_wait_with_deadline() and nsync_mu_wait_with_deadline() calls use an
   absolute rather than a relative timeout.  This is less error prone, as
   described in the comment on nsync_cv_wait_with_deadline().  Alas, relative
   timeouts are seductive in trivial examples (such as tests).  These are the
   first things that people try, so they are likely to be requested.  If enough
   people complain we could give them that particular piece of rope.

   Excessive evaluations of the same wait condition are avoided by maintaining
   waiter.same_condition as a doubly-linked list of waiters with the same
   non-NULL wait condition that are also adjacent in the waiter list.  This does
   well even with large numbers of threads if there is at most one
   wait condition that can be false at any given time (such as in a
   producer/consumer queue, which cannot be both empty and full
   simultaneously).  One could imagine a queueing mechanism that would
   guarantee to evaluate each condition at most once per wakeup, but that would
   be substantially more complex, and would still degrade if the number of
   distinct wakeup conditions were high.  So clients are advised to resort to
   condition variables if they have many distinct wakeup conditions. */

/* Used in spinloops to delay resumption of the loop.
   Usage:
       unsigned attempts = 0;
       while (try_something) {
	  attempts = nsync_spin_delay_ (attempts);
       } */
unsigned nsync_spin_delay_ (unsigned attempts) {
	if (attempts < 7) {
		volatile int i;
		for (i = 0; i != 1 << attempts; i++) {
		}
		attempts++;
	} else {
		nsync_yield_ ();
	}
	return (attempts);
}

/* Spin until (*w & test) == 0, then atomically perform *w = ((*w | set) &
   ~clear), perform an acquire barrier, and return the previous value of *w.
   */
uint32_t nsync_spin_test_and_set_ (nsync_atomic_uint32_ *w, uint32_t test,
				   uint32_t set, uint32_t clear) {
	unsigned attempts = 0; /* CV_SPINLOCK retry count */
	uint32_t old = ATM_LOAD (w);
	while ((old & test) != 0 || !ATM_CAS_ACQ (w, old, (old | set) & ~clear)) {
		attempts = nsync_spin_delay_ (attempts);
		old = ATM_LOAD (w);
	}
	return (old);
}

/* ====================================================================================== */

struct nsync_waiter_s *nsync_dll_nsync_waiter_ (nsync_dll_element_ *e) {
	struct nsync_waiter_s *nw = (struct nsync_waiter_s *) e->container;
	ASSERT (nw->tag == NSYNC_WAITER_TAG);
	ASSERT (e == &nw->q);
	return (nw);
}
waiter *nsync_dll_waiter_ (nsync_dll_element_ *e) {
	struct nsync_waiter_s *nw = DLL_NSYNC_WAITER (e);
	waiter *w = CONTAINER (waiter, nw, nw);
	ASSERT ((nw->flags & NSYNC_WAITER_FLAG_MUCV) != 0);
	ASSERT (w->tag == WAITER_TAG);
	ASSERT (e == &w->nw.q);
	return (w);
}

waiter *nsync_dll_waiter_samecond_ (nsync_dll_element_ *e) {
	waiter *w = (waiter *) e->container;
	ASSERT (w->tag == WAITER_TAG);
	ASSERT (e == &w->same_condition);
	return (w);
}

/* -------------------------------- */

static nsync_dll_list_ free_waiters = NULL;

/* free_waiters points to a doubly-linked list of free waiter structs. */
static nsync_atomic_uint32_ free_waiters_mu; /* spinlock; protects free_waiters */

static THREAD_LOCAL waiter *waiter_for_thread;
static void waiter_destroy (void *v) {
	waiter *w = (waiter *) v;
	/* Reset waiter_for_thread in case another thread-local variable reuses
	   the waiter in its destructor while the waiter is taken by the other
	   thread from free_waiters. This can happen as the destruction order
	   of thread-local variables can be arbitrary in some platform e.g.
	   POSIX.  */
	waiter_for_thread = NULL;
	IGNORE_RACES_START ();
	ASSERT ((w->flags & (WAITER_RESERVED|WAITER_IN_USE)) == WAITER_RESERVED);
	w->flags &= ~WAITER_RESERVED;
	nsync_spin_test_and_set_ (&free_waiters_mu, 1, 1, 0);
	free_waiters = nsync_dll_make_first_in_list_ (free_waiters, &w->nw.q);
	ATM_STORE_REL (&free_waiters_mu, 0); /* release store */
	IGNORE_RACES_END ();
}

/* If non-nil, nsync_malloc_ptr_ points to a malloc-like routine that allocated
   memory, used by mutex and condition variable code to allocate waiter
   structs.  This would allow nsync's mutexes to be used inside an
   implementation of malloc(), by providing another, simpler allocator here.
   The intent is that the implicit NULL value here can be overridden by a
   client declaration that uses an initializer.  */
void *(*nsync_malloc_ptr_) (size_t size);

/* Return a pointer to an unused waiter struct.
   Ensures that the enclosed timer is stopped and its channel drained. */
waiter *nsync_waiter_new_ (void) {
	nsync_dll_element_ *q;
	waiter *tw;
	waiter *w;
	if (HAVE_THREAD_LOCAL) {
		tw = waiter_for_thread;
	} else {
		tw = (waiter *) nsync_per_thread_waiter_ (&waiter_destroy);
	}
	w = tw;
	if (w == NULL || (w->flags & (WAITER_RESERVED|WAITER_IN_USE)) != WAITER_RESERVED) {
		w = NULL;
		nsync_spin_test_and_set_ (&free_waiters_mu, 1, 1, 0);
		q = nsync_dll_first_ (free_waiters);
		if (q != NULL) { /* If free list is non-empty, dequeue an item. */
			free_waiters = nsync_dll_remove_ (free_waiters, q);
			w = DLL_WAITER (q);
		}
		ATM_STORE_REL (&free_waiters_mu, 0); /* release store */
		if (w == NULL) { /* If free list was empty, allocate an item. */
			if (nsync_malloc_ptr_ != NULL) { /* Use client's malloc() */
				w = (waiter *) (*nsync_malloc_ptr_) (sizeof (*w));
			} else {  /* standard malloc () */
				w = (waiter *) malloc (sizeof (*w));
			}
			w->tag = WAITER_TAG;
			w->nw.tag = NSYNC_WAITER_TAG;
			nsync_mu_semaphore_init (&w->sem);
			w->nw.sem = &w->sem;
			nsync_dll_init_ (&w->nw.q, &w->nw);
			NSYNC_ATOMIC_UINT32_STORE_ (&w->nw.waiting, 0);
			w->nw.flags = NSYNC_WAITER_FLAG_MUCV;
			ATM_STORE (&w->remove_count, 0);
			nsync_dll_init_ (&w->same_condition, w);
			w->flags = 0;
		}
		if (tw == NULL) {
			w->flags |= WAITER_RESERVED;
			nsync_set_per_thread_waiter_ (w, &waiter_destroy);
			if (HAVE_THREAD_LOCAL) {
				waiter_for_thread = w;
			}
		}
	}
	w->flags |= WAITER_IN_USE;
	return (w);
}

/* Return an unused waiter struct *w to the free pool. */
void nsync_waiter_free_ (waiter *w) {
	ASSERT ((w->flags & WAITER_IN_USE) != 0);
	w->flags &= ~WAITER_IN_USE;
	if ((w->flags & WAITER_RESERVED) == 0) {
		nsync_spin_test_and_set_ (&free_waiters_mu, 1, 1, 0);
		free_waiters = nsync_dll_make_first_in_list_ (free_waiters, &w->nw.q);
		ATM_STORE_REL (&free_waiters_mu, 0); /* release store */
	}
}

/* ====================================================================================== */

/* writer_type points to a lock_type that describes how to manipulate a mu for a writer. */
static lock_type Xwriter_type = {
	MU_WZERO_TO_ACQUIRE,
	MU_WADD_TO_ACQUIRE,
	MU_WHELD_IF_NON_ZERO,
	MU_WSET_WHEN_WAITING,
	MU_WCLEAR_ON_ACQUIRE,
	MU_WCLEAR_ON_UNCONTENDED_RELEASE
};
lock_type *nsync_writer_type_ = &Xwriter_type;


/* reader_type points to a lock_type that describes how to manipulate a mu for a reader. */
static lock_type Xreader_type = {
	MU_RZERO_TO_ACQUIRE,
	MU_RADD_TO_ACQUIRE,
	MU_RHELD_IF_NON_ZERO,
	MU_RSET_WHEN_WAITING,
	MU_RCLEAR_ON_ACQUIRE,
	MU_RCLEAR_ON_UNCONTENDED_RELEASE
};
lock_type *nsync_reader_type_ = &Xreader_type;

NSYNC_CPP_END_
