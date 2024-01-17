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

#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync.h"
#include "dll.h"
#include "sem.h"
#include "wait_internal.h"
#include "common.h"
#include "atomic.h"

NSYNC_CPP_START_

/* Initialize *mu. */
void nsync_mu_init (nsync_mu *mu) {
	memset ((void *) mu, 0, sizeof (*mu));
	RWLOCK_CREATE (mu);
}

/* Release the mutex spinlock. */
static void mu_release_spinlock (nsync_mu *mu) {
	uint32_t old_word = ATM_LOAD (&mu->word);
	while (!ATM_CAS_REL (&mu->word, old_word, old_word & ~MU_SPINLOCK)) {
		old_word = ATM_LOAD (&mu->word);
	}
}

/* Lock *mu using the specified lock_type, waiting on *w if necessary.
   "clear" should be zero if the thread has not previously slept on *mu, and
   MU_DESIG_WAKER if it has; this represents bits that nsync_mu_lock_slow_() must clear when
   it either acquires or sleeps on *mu.  The caller owns *w on return; it is in a valid
   state to be returned to the free pool. */
void nsync_mu_lock_slow_ (nsync_mu *mu, waiter *w, uint32_t clear, lock_type *l_type) {
	uint32_t zero_to_acquire;
	uint32_t wait_count;
	uint32_t long_wait;
	unsigned attempts = 0; /* attempt count; used for spinloop backoff */
	w->cv_mu = NULL;      /* not a cv wait */
	w->cond.f = NULL; /* Not using a conditional critical section. */
	w->cond.v = NULL;
	w->cond.eq = NULL;
	w->l_type = l_type;
	zero_to_acquire = l_type->zero_to_acquire;
	if (clear != 0) {
		/* Only the constraints of mutual exclusion should stop a designated waker. */
		zero_to_acquire &= ~(MU_WRITER_WAITING | MU_LONG_WAIT);
	}
	wait_count = 0; /* number of times we waited, and were woken. */
	long_wait = 0; /* set to MU_LONG_WAIT when wait_count gets large */
	for (;;) {
		uint32_t old_word = ATM_LOAD (&mu->word);
		if ((old_word & zero_to_acquire) == 0) {
			/* lock can be acquired; try to acquire, possibly
			   clearing MU_DESIG_WAKER and MU_LONG_WAIT.  */
			if (ATM_CAS_ACQ (&mu->word, old_word,
					 (old_word+l_type->add_to_acquire) &
					  ~(clear|long_wait|l_type->clear_on_acquire))) {
				return;
			}
		} else if ((old_word&MU_SPINLOCK) == 0 &&
			   ATM_CAS_ACQ (&mu->word, old_word,
					(old_word|MU_SPINLOCK|long_wait|
					 l_type->set_when_waiting) & ~(clear | MU_ALL_FALSE))) {

			/* Spinlock is now held, and lock is held by someone
			   else; MU_WAITING has also been set; queue ourselves.
			   There's no need to adjust same_condition here,
			   because w.condition==NULL.  */
			ATM_STORE (&w->nw.waiting, 1);
			if (wait_count == 0) {
				/* first wait goes to end of queue */
				mu->waiters = nsync_dll_make_last_in_list_ (mu->waiters,
								            &w->nw.q);
			} else {
				/* subsequent waits go to front of queue */
				mu->waiters = nsync_dll_make_first_in_list_ (mu->waiters,
								             &w->nw.q);
			}

			/* Release spinlock.  Cannot use a store here, because
			   the current thread does not hold the mutex.  If
			   another thread were a designated waker, the mutex
			   holder could be concurrently unlocking, even though
			   we hold the spinlock. */
			mu_release_spinlock (mu);

			/* wait until awoken. */
			while (ATM_LOAD_ACQ (&w->nw.waiting) != 0) { /* acquire load */
				nsync_mu_semaphore_p (&w->sem);
			}
			wait_count++;
			/* If the thread has been woken more than this many
			   times, and still not acquired, it sets the
			   MU_LONG_WAIT bit to prevent thread that have not
			   waited from acquiring.  This is the starvation
			   avoidance mechanism.  The number is fairly high so
			   that we continue to benefit from the throughput of
			   not having running threads wait unless absolutely
			   necessary.  */
			if (wait_count == LONG_WAIT_THRESHOLD) { /* repeatedly woken */
				long_wait = MU_LONG_WAIT; /* force others to wait at least once */
			}

			attempts = 0;
			clear = MU_DESIG_WAKER;
			/* Threads that have been woken at least once don't care
			   about waiting writers or long waiters. */
			zero_to_acquire &= ~(MU_WRITER_WAITING | MU_LONG_WAIT);
		}
		attempts = nsync_spin_delay_ (attempts);
	}
}

/* Attempt to acquire *mu in writer mode without blocking, and return non-zero
   iff successful.  Return non-zero with high probability if *mu was free on
   entry.  */
int nsync_mu_trylock (nsync_mu *mu) {
	int result;
	IGNORE_RACES_START ();
	if (ATM_CAS_ACQ (&mu->word, 0, MU_WADD_TO_ACQUIRE)) { /* acquire CAS */
		result = 1;
	} else {
		uint32_t old_word = ATM_LOAD (&mu->word);
		result = ((old_word & MU_WZERO_TO_ACQUIRE) == 0 &&
			  ATM_CAS_ACQ (&mu->word, old_word,
				       (old_word + MU_WADD_TO_ACQUIRE) & ~MU_WCLEAR_ON_ACQUIRE));
	}
	IGNORE_RACES_END ();
	RWLOCK_TRYACQUIRE (result, mu, 1);
	return (result);
}

/* Block until *mu is free and then acquire it in writer mode. */
void nsync_mu_lock (nsync_mu *mu) {
	IGNORE_RACES_START ();
	if (!ATM_CAS_ACQ (&mu->word, 0, MU_WADD_TO_ACQUIRE)) { /* acquire CAS */
		uint32_t old_word = ATM_LOAD (&mu->word);
		if ((old_word&MU_WZERO_TO_ACQUIRE) != 0 ||
		    !ATM_CAS_ACQ (&mu->word, old_word,
				  (old_word+MU_WADD_TO_ACQUIRE) & ~MU_WCLEAR_ON_ACQUIRE)) {
			waiter *w = nsync_waiter_new_ ();
			nsync_mu_lock_slow_ (mu, w, 0, nsync_writer_type_);
			nsync_waiter_free_ (w);
		}
	}
	IGNORE_RACES_END ();
	RWLOCK_TRYACQUIRE (1, mu, 1);
}

/* Attempt to acquire *mu in reader mode without blocking, and return non-zero
   iff successful.  Returns non-zero with high probability if *mu was free on
   entry.  It may fail to acquire if a writer is waiting, to avoid starvation.
   */
int nsync_mu_rtrylock (nsync_mu *mu) {
	int result;
	IGNORE_RACES_START ();
	if (ATM_CAS_ACQ (&mu->word, 0, MU_RADD_TO_ACQUIRE)) { /* acquire CAS */
		result = 1;
	} else {
		uint32_t old_word = ATM_LOAD (&mu->word);
		result = ((old_word&MU_RZERO_TO_ACQUIRE) == 0 &&
			  ATM_CAS_ACQ (&mu->word, old_word,
				       (old_word+MU_RADD_TO_ACQUIRE) & ~MU_RCLEAR_ON_ACQUIRE));
	}
	IGNORE_RACES_END ();
	RWLOCK_TRYACQUIRE (result, mu, 0);
	return (result);
}

/* Block until *mu can be acquired in reader mode and then acquire it. */
void nsync_mu_rlock (nsync_mu *mu) {
	IGNORE_RACES_START ();
	if (!ATM_CAS_ACQ (&mu->word, 0, MU_RADD_TO_ACQUIRE)) { /* acquire CAS */
		uint32_t old_word = ATM_LOAD (&mu->word);
		if ((old_word&MU_RZERO_TO_ACQUIRE) != 0 ||
		    !ATM_CAS_ACQ (&mu->word, old_word,
				  (old_word+MU_RADD_TO_ACQUIRE) & ~MU_RCLEAR_ON_ACQUIRE)) {
			waiter *w = nsync_waiter_new_ ();
			nsync_mu_lock_slow_ (mu, w, 0, nsync_reader_type_);
			nsync_waiter_free_ (w);
		}
	}
	IGNORE_RACES_END ();
	RWLOCK_TRYACQUIRE (1, mu, 0);
}

/* Invoke the condition associated with *p, which is an element of
   a "waiter" list. */
static int condition_true (nsync_dll_element_ *p) {
	return ((*DLL_WAITER (p)->cond.f) (DLL_WAITER (p)->cond.v));
}

/* If *p is an element of waiter_list (a list of "waiter" structs(, return a
   pointer to the next element of the list that has a different condition. */
static nsync_dll_element_ *skip_past_same_condition (
	nsync_dll_list_ waiter_list, nsync_dll_element_ *p) {
	nsync_dll_element_ *next;
	nsync_dll_element_ *last_with_same_condition =
		&DLL_WAITER_SAMECOND (DLL_WAITER (p)->same_condition.prev)->nw.q;
	if (last_with_same_condition != p && last_with_same_condition != p->prev) {
		/* First in set with same condition, so skip to end.  */
		next = nsync_dll_next_ (waiter_list, last_with_same_condition);
	} else {
		next = nsync_dll_next_ (waiter_list, p);
	}
	return (next);
}

/* Merge the same_condition lists of *p and *n if they have the same non-NULL
   condition.  */
void nsync_maybe_merge_conditions_ (nsync_dll_element_ *p, nsync_dll_element_ *n) {
	if (p != NULL && n != NULL &&
	    WAIT_CONDITION_EQ (&DLL_WAITER (p)->cond, &DLL_WAITER (n)->cond)) {
		nsync_dll_splice_after_ (&DLL_WAITER (p)->same_condition,
				  &DLL_WAITER (n)->same_condition);
	}
}

/* Remove element *e from nsync_mu waiter queue mu_queue, fixing
   up the same_condition list by merging the lists on either side if possible.
   Also increment the waiter's remove_count. */
nsync_dll_list_ nsync_remove_from_mu_queue_ (nsync_dll_list_ mu_queue, nsync_dll_element_ *e) {
	/* Record previous and next elements in the original queue. */
	nsync_dll_element_ *prev = e->prev;
	nsync_dll_element_ *next = e->next;
	uint32_t old_value;
	/* Remove. */
	mu_queue = nsync_dll_remove_ (mu_queue, e);
        do {    
                old_value = ATM_LOAD (&DLL_WAITER (e)->remove_count);
        } while (!ATM_CAS (&DLL_WAITER (e)->remove_count, old_value, old_value+1));
	if (!nsync_dll_is_empty_ (mu_queue)) {
		/* Fix up same_condition. */
		nsync_dll_element_ *e_same_condition = &DLL_WAITER (e)->same_condition;

		if (e_same_condition->next != e_same_condition) {
			/* *e is linked to a same_condition neighbour---just remove it. */
			e_same_condition->next->prev = e_same_condition->prev;
			e_same_condition->prev->next = e_same_condition->next;
			e_same_condition->next = e_same_condition;
			e_same_condition->prev = e_same_condition;
		} else if (prev != nsync_dll_last_ (mu_queue)) {
			/* Merge the new neighbours together if we can. */
			nsync_maybe_merge_conditions_ (prev, next);
		}
	}
	return (mu_queue);
}

/* Unlock *mu and wake one or more waiters as appropriate after an unlock.
   It is called with *mu held in mode l_type. */
void nsync_mu_unlock_slow_ (nsync_mu *mu, lock_type *l_type) {
	unsigned attempts = 0; /* attempt count; used for backoff */
	for (;;) {
		uint32_t old_word = ATM_LOAD (&mu->word);
		int testing_conditions = ((old_word & MU_CONDITION) != 0);
		uint32_t early_release_mu = l_type->add_to_acquire;
		uint32_t late_release_mu = 0;
		if (testing_conditions) {
			/* Convert to a writer lock, and release later.
			   - A writer lock is currently needed to test conditions
			     because exclusive access is needed to the list to
			     allow modification.  The spinlock cannot be used
			     to achieve that, because an internal lock should not
			     be held when calling the external predicates.
			   - We must test conditions even though a reader region
			     cannot have made any new ones true because some
			     might have been true before the reader region started.
			     The MU_ALL_FALSE test below shortcuts the case where
			     the conditions are known all to be false.  */
			early_release_mu = l_type->add_to_acquire - MU_WLOCK;
			late_release_mu = MU_WLOCK;
		}
		if ((old_word&MU_WAITING) == 0 || (old_word&MU_DESIG_WAKER) != 0 ||
		    (old_word & MU_RLOCK_FIELD) > MU_RLOCK ||
		    (old_word & (MU_RLOCK|MU_ALL_FALSE)) == (MU_RLOCK|MU_ALL_FALSE)) {
			/* no one to wake, there's a designated waker waking
			   up, there are still readers, or it's a reader and all waiters
			   have false conditions */
			if (ATM_CAS_REL (&mu->word, old_word,
					 (old_word - l_type->add_to_acquire) &
					 ~l_type->clear_on_uncontended_release)) {
				return;
			}
		} else if ((old_word&MU_SPINLOCK) == 0 &&
			   ATM_CAS_RELACQ (&mu->word, old_word,
                                           (old_word-early_release_mu)|MU_SPINLOCK|MU_DESIG_WAKER)) {
			nsync_dll_list_ wake;
			lock_type *wake_type;
			uint32_t clear_on_release;
			uint32_t set_on_release;
			/* The spinlock is now held, and we've set the
			   designated wake flag, since we're likely to wake a
			   thread that will become that designated waker.  If
			   there are conditions to check, the mutex itself is
			   still held.  */

			nsync_dll_element_ *p = NULL;
			nsync_dll_element_ *next = NULL;

			/* Swap the entire mu->waiters list into the local
			   "new_waiters" list.  This gives us exclusive access
			   to the list, even if we unlock the spinlock, which
			   we may do if checking conditions.  The loop below
			   will grab more new waiters that arrived while we
			   were checking conditions, and terminates only if no
			   new waiters arrive in one loop iteration.  */
			nsync_dll_list_ waiters = NULL;
			nsync_dll_list_ new_waiters = mu->waiters;
			mu->waiters = NULL;

			/* Remove a waiter from the queue, if possible. */
			wake = NULL;       /* waiters to wake. */
			wake_type = NULL; /* type of waiter(s) on wake, or NULL if wake is empty. */
			clear_on_release = MU_SPINLOCK;
			set_on_release = MU_ALL_FALSE;
			while (!nsync_dll_is_empty_ (new_waiters)) { /* some new waiters to consider */
				p = nsync_dll_first_ (new_waiters);
				if (testing_conditions) {
					/* Should we continue to test conditions? */
					if (wake_type == nsync_writer_type_) {
						/* No, because we're already waking a writer,
						   and need wake no others.*/
						testing_conditions = 0;
					} else if (wake_type == NULL &&
						DLL_WAITER (p)->l_type != nsync_reader_type_ &&
						DLL_WAITER (p)->cond.f == NULL) {
						/* No, because we've woken no one, but the
						   first waiter is a writer with no condition,
						   so we will certainly wake it, and need wake
						   no others. */
						testing_conditions = 0;
					}
				}
				/* If testing waiters' conditions, release the
				   spinlock while still holding the write lock.
				   This is so that the spinlock is not held
				   while the conditions are evaluated.  */
				if (testing_conditions) {
					mu_release_spinlock (mu);
				}

				/* Process the new waiters picked up in this iteration of the
				   "while (!nsync_dll_is_empty_ (new_waiters))" loop,
				   and stop looking when we run out of waiters, or we find
				   a writer to wake up. */
				while (p != NULL && wake_type != nsync_writer_type_) {
					int p_has_condition;
					next = nsync_dll_next_ (new_waiters, p);
					p_has_condition = (DLL_WAITER (p)->cond.f != NULL);
					if (p_has_condition && !testing_conditions) {
						nsync_panic_ ("checking a waiter condition "
							      "while unlocked\n");
					}
					if (p_has_condition && !condition_true (p)) {
						/* condition is false */
						/* skip to the end of the same_condition group. */
						next = skip_past_same_condition (new_waiters, p);
					} else if (wake_type == NULL ||
						   DLL_WAITER (p)->l_type == nsync_reader_type_) {
						/* Wake this thread. */
						new_waiters = nsync_remove_from_mu_queue_ (
							new_waiters, p);
						wake = nsync_dll_make_last_in_list_ (wake, p);
						wake_type = DLL_WAITER (p)->l_type;
					} else {
						/* Failing to wake a writer
						   that could acquire if it
						   were first.  */
						set_on_release |= MU_WRITER_WAITING;
						set_on_release &= ~MU_ALL_FALSE;
					}
					p = next;
				}

				if (p != NULL) {
					/* Didn't search to end of list, so can't be sure
					   all conditions are false. */
					set_on_release &= ~MU_ALL_FALSE;
				}

				/* If testing waiters' conditions, reacquire the spinlock
				   released above. */
				if (testing_conditions) {
					nsync_spin_test_and_set_ (&mu->word, MU_SPINLOCK,
								  MU_SPINLOCK, 0);
				}

				/* add the new_waiters to the last of the waiters. */
				nsync_maybe_merge_conditions_ (nsync_dll_last_ (waiters),
							       nsync_dll_first_ (new_waiters));
				waiters = nsync_dll_make_last_in_list_ (waiters,
								 nsync_dll_last_ (new_waiters));
				/* Pick up the next set of new waiters. */
				new_waiters = mu->waiters;
				mu->waiters = NULL;
			}

			/* Return the local waiter list to *mu. */
			mu->waiters = waiters;

			if (nsync_dll_is_empty_ (wake)) {
				/* not waking a waiter => no designated waker */
				clear_on_release |= MU_DESIG_WAKER;
			}

			if ((set_on_release & MU_ALL_FALSE) == 0) {
				/* If not explicitly setting MU_ALL_FALSE, clear it. */
				clear_on_release |= MU_ALL_FALSE;
			}

			if (nsync_dll_is_empty_ (mu->waiters)) {
				/* no waiters left */
				clear_on_release |= MU_WAITING | MU_WRITER_WAITING |
						    MU_CONDITION | MU_ALL_FALSE;
			}

			/* Release the spinlock, and possibly the lock if
			   late_release_mu is non-zero.  Other bits are set or
			   cleared according to whether we woke any threads,
			   whether any waiters remain, and whether any of them
			   are writers.  */
			old_word = ATM_LOAD (&mu->word);
			while (!ATM_CAS_REL (&mu->word, old_word,
					     ((old_word-late_release_mu)|set_on_release) &
					     ~clear_on_release)) { /* release CAS */
				old_word = ATM_LOAD (&mu->word);
			}
			/* Wake the waiters. */
			for (p = nsync_dll_first_ (wake); p != NULL; p = next) {
				next = nsync_dll_next_ (wake, p);
				wake = nsync_dll_remove_ (wake, p);
				ATM_STORE_REL (&DLL_NSYNC_WAITER (p)->waiting, 0);
				nsync_mu_semaphore_v (&DLL_WAITER (p)->sem);
			}
			return;
		}
		attempts = nsync_spin_delay_ (attempts);
	}
}

/* Unlock *mu, which must be held in write mode, and wake waiters, if appropriate. */
void nsync_mu_unlock (nsync_mu *mu) {
	RWLOCK_RELEASE (mu, 1);
	IGNORE_RACES_START ();
	/* C is not a garbage-collected language, so we cannot release until we
	   can be sure that we will not have to touch the mutex again to wake a
	   waiter.  Another thread could acquire, decrement a reference count
	   and deallocate the mutex before the current thread touched the mutex
	   word again. */
	if (!ATM_CAS_REL (&mu->word, MU_WLOCK, 0)) {
		uint32_t old_word = ATM_LOAD (&mu->word);
                /* Clear MU_ALL_FALSE because the critical section we're just
                   leaving may have made some conditions true.  */
		uint32_t new_word = (old_word - MU_WLOCK) & ~MU_ALL_FALSE;
                /* Sanity check:  mutex must be held in write mode, and there
                   must be no readers.  */
		if ((new_word & (MU_RLOCK_FIELD | MU_WLOCK)) != 0) {
			if ((old_word & MU_RLOCK_FIELD) != 0) {
				nsync_panic_ ("attempt to nsync_mu_unlock() an nsync_mu "
				       "held in read mode\n");
			} else {
				nsync_panic_ ("attempt to nsync_mu_unlock() an nsync_mu "
				       "not held in write mode\n");
			}
		} else if ((old_word & (MU_WAITING|MU_DESIG_WAKER)) == MU_WAITING ||
			   !ATM_CAS_REL (&mu->word, old_word, new_word)) {
			/* There are waiters and no designated waker, or
			   our initial CAS attempt failed, to use slow path. */
			nsync_mu_unlock_slow_ (mu, nsync_writer_type_);
		}
	}
	IGNORE_RACES_END ();
}

/* Unlock *mu, which must be held in read mode, and wake waiters, if appropriate. */
void nsync_mu_runlock (nsync_mu *mu) {
	RWLOCK_RELEASE (mu, 0);
	IGNORE_RACES_START ();
	/* See comment in nsync_mu_unlock(). */
	if (!ATM_CAS_REL (&mu->word, MU_RLOCK, 0)) {
		uint32_t old_word = ATM_LOAD (&mu->word);
                /* Sanity check:  mutex must not be held in write mode and
                   reader count must not be 0.  */
		if (((old_word ^ MU_WLOCK) & (MU_WLOCK | MU_RLOCK_FIELD)) == 0) {
			if ((old_word & MU_WLOCK) != 0) {
				nsync_panic_ ("attempt to nsync_mu_runlock() an nsync_mu "
				       "held in write mode\n");
			} else {
				nsync_panic_ ("attempt to nsync_mu_runlock() an nsync_mu "
				       "not held in read mode\n");
			}
		} else if ((old_word & (MU_WAITING | MU_DESIG_WAKER)) == MU_WAITING &&
			    (old_word & (MU_RLOCK_FIELD|MU_ALL_FALSE)) == MU_RLOCK) {
                        /* There are waiters and no designated waker, the last
                           reader is unlocking, and not all waiters have a
                           false condition.  So we must take the slow path to
                           attempt to wake a waiter.  */
			nsync_mu_unlock_slow_ (mu, nsync_reader_type_);
		} else if (!ATM_CAS_REL (&mu->word, old_word, old_word - MU_RLOCK)) {
			/* CAS attempt failed, so take slow path. */
			nsync_mu_unlock_slow_ (mu, nsync_reader_type_);
		}
	}
	IGNORE_RACES_END ();
}

/* Abort if *mu is not held in write mode. */
void nsync_mu_assert_held (const nsync_mu *mu) {
	IGNORE_RACES_START ();
	if ((ATM_LOAD (&mu->word) & MU_WHELD_IF_NON_ZERO) == 0) {
		nsync_panic_ ("nsync_mu not held in write mode\n");
	}
	IGNORE_RACES_END ();
}

/* Abort if *mu is not held in read or write mode. */
void nsync_mu_rassert_held (const nsync_mu *mu) {
	IGNORE_RACES_START ();
	if ((ATM_LOAD (&mu->word) & MU_ANY_LOCK) == 0) {
		nsync_panic_ ("nsync_mu not held in some mode\n");
	}
	IGNORE_RACES_END ();
}

/* Return whether *mu is held in read mode.
   Requires that *mu is held in some mode. */
int nsync_mu_is_reader (const nsync_mu *mu) {
	uint32_t word;
	IGNORE_RACES_START ();
	word = ATM_LOAD (&mu->word);
	if ((word & MU_ANY_LOCK) == 0) {
		nsync_panic_ ("nsync_mu not held in some mode\n");
	}
	IGNORE_RACES_END ();
	return ((word & MU_WLOCK) == 0);
}

NSYNC_CPP_END_
