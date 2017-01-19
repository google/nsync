#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync.h"
#include "dll.h"
#include "sem.h"
#include "common.h"
#include "atomic.h"
#include "time_internal.h"

NSYNC_CPP_START_

/* Set the expiry time in *n to t */
static void set_expiry_time (nsync_note *n, nsync_time t) {
	n->expiry_time = t;
	n->expiry_time_valid = 1;
}

/* Return a pointer to the note containing nsync_dll_element_ *e. */
#define DLL_NOTE(e) ((nsync_note *)((e)->container))

/* Return whether n->children is empty.  Assumes n->note_mu held. */
static int no_children (void *v) {
	return (nsync_dll_is_empty_ (((nsync_note *)v)->children));
}

#define WAIT_FOR_NO_CHILDREN(pred_, n_) nsync_mu_wait (&(n_)->note_mu, &pred_, (n_))
#define WAKEUP_NO_CHILDREN(n_) do { } while (0)

/*
#define WAIT_FOR_NO_CHILDREN(pred_, n_) do { \
		while (!pred_ (n_)) { nsync_cv_wait (&(n_)->no_children_cv, &(n_)->note_mu); } \
	} while (0)
#define WAKEUP_NO_CHILDREN(n_) nsync_cv_broadcast (&(n_)->no_children_cv)
*/

/* Notify *n and all its descendants that are not already disconnnecting.
   n->note_mu is held.  May release and reacquire n->note_mu.
   parent->note_mu is held if parent != NULL. */
static void note_notify_child (nsync_note *n, nsync_note *parent) {
	if (nsync_time_cmp (NOTIFIED_TIME (n), nsync_time_zero) > 0) {
		nsync_dll_element_ *p;
		nsync_dll_element_ *next;
		ATM_STORE_REL (&n->notified, 1);
		while ((p = nsync_dll_first_ (n->waiters)) != NULL) {
			struct nsync_waiter_s *nw = DLL_NSYNC_WAITER (p);
			n->waiters = nsync_dll_remove_ (n->waiters, p);
			ATM_STORE (&nw->waiting, 1);
			nsync_mu_semaphore_v (nw->sem);
		}
		for (p = nsync_dll_first_ (n->children); p != NULL; p = next) {
			nsync_note *child = DLL_NOTE (p);
			next = nsync_dll_next_ (n->children, p);
			nsync_mu_lock (&child->note_mu);
			if (child->disconnecting == 0) {
				note_notify_child (child, n);
			}
			nsync_mu_unlock (&child->note_mu);
		}
		WAIT_FOR_NO_CHILDREN (no_children, n);
		if (parent != NULL) {
			parent->children = nsync_dll_remove_ (parent->children,
						       (nsync_dll_element_ *) &n->parent_child_link);
			WAKEUP_NO_CHILDREN (parent);
			n->parent = NULL;
		}
	}
}

/* Notify *n and all its descendants that are not already disconnnecting.
   No locks are held. */
static void notify (nsync_note *n) {
	nsync_mu_lock (&n->note_mu);
	if (nsync_time_cmp (NOTIFIED_TIME (n), nsync_time_zero) > 0) {
		nsync_note *parent;
		n->disconnecting++;
		parent = n->parent;
		if (parent != NULL && !nsync_mu_trylock (&parent->note_mu)) {
			nsync_mu_unlock (&n->note_mu);
			nsync_mu_lock (&parent->note_mu);
			nsync_mu_lock (&n->note_mu);
		}
		note_notify_child (n, parent);
		if (parent != NULL) {
			nsync_mu_unlock (&parent->note_mu);
		}
		n->disconnecting--;
	}
	nsync_mu_unlock (&n->note_mu);
}

/* Return the deadline by which *n is certain to be notified,
   setting it to zero if it already has passed that time.
   Requires n->note_mu not held on entry. */
nsync_time nsync_note_notified_deadline_ (nsync_note *n) {
	nsync_time ntime;
	if (ATM_LOAD_ACQ (&n->notified) != 0) {
		ntime = nsync_time_zero;
	} else {
		nsync_mu_lock (&n->note_mu);
		ntime = NOTIFIED_TIME (n);
		nsync_mu_unlock (&n->note_mu);
		if (nsync_time_cmp (ntime, nsync_time_zero) > 0) {
			if (nsync_time_cmp (ntime, nsync_time_now ()) <= 0) {
				notify (n);
				ntime = nsync_time_zero;
			}
		}
	}
	return (ntime);
}

int nsync_note_is_notified (nsync_note *n) {
	int result;
	IGNORE_RACES_START ();
	result = (nsync_time_cmp (nsync_note_notified_deadline_ (n), nsync_time_zero) <= 0);
	IGNORE_RACES_END ();
	return (result);
}

static nsync_note *assert_parent_child_link_size = (nsync_note *)(uintptr_t)(1 /
	(sizeof (assert_parent_child_link_size->parent_child_link) >= sizeof (nsync_dll_element_)));

nsync_note *nsync_note_init (nsync_note *n, nsync_note *parent,
			     nsync_time abs_deadline) {
	IGNORE_RACES_START ();
	memset (n, 0, sizeof (*n));
	nsync_dll_init_ ((nsync_dll_element_ *)&n->parent_child_link, n);
	set_expiry_time (n, abs_deadline);
	if (!nsync_note_is_notified (n) && parent != NULL) {
		nsync_time parent_time;
		nsync_mu_lock (&parent->note_mu);
		parent_time = NOTIFIED_TIME (parent);
		if (nsync_time_cmp (parent_time, abs_deadline) < 0) {
			set_expiry_time (n, parent_time);
		}
		if (nsync_time_cmp (parent_time, nsync_time_zero) > 0) {
			n->parent = parent;
			parent->children = nsync_dll_make_last_in_list_ (parent->children,
				(nsync_dll_element_ *)&n->parent_child_link);
		}
		nsync_mu_unlock (&parent->note_mu);
	}
	IGNORE_RACES_END ();
	return (n);
}

void nsync_note_notify (nsync_note *n) {
	IGNORE_RACES_START ();
	if (nsync_time_cmp (nsync_note_notified_deadline_ (n), nsync_time_zero) > 0) {
		notify (n);
	}
	IGNORE_RACES_END ();
}

int nsync_note_wait (nsync_note *n, nsync_time abs_deadline) {
	struct nsync_waitable_s waitable;
	struct nsync_waitable_s *pwaitable = &waitable;
	waitable.v = n;
	waitable.funcs = &nsync_note_waitable_funcs;
	return (nsync_wait_n (NULL, NULL, NULL, abs_deadline, 1, &pwaitable) == 0);
}

static nsync_time note_ready_time (void *v, struct nsync_waiter_s *nw UNUSED) {
	return (nsync_note_notified_deadline_ ((nsync_note *)v));
}

static nsync_time note_enqueue (void *v, struct nsync_waiter_s *nw) {
	nsync_note *n = (nsync_note *) v;
	nsync_time ntime;
	nsync_mu_lock (&n->note_mu);
	ATM_STORE (&nw->waiting, 0);
	ntime = NOTIFIED_TIME (n);
	if (nsync_time_cmp (ntime, nsync_time_zero) > 0) {
		n->waiters = nsync_dll_make_last_in_list_ (n->waiters, (nsync_dll_element_ *)nw->q);
		ATM_STORE (&nw->waiting, 1);
	}
	nsync_mu_unlock (&n->note_mu);
	return (ntime);
}

static nsync_time note_dequeue (void *v, struct nsync_waiter_s *nw) {
	nsync_note *n = (nsync_note *) v;
	nsync_time ntime;
	nsync_note_notified_deadline_ (n);
	nsync_mu_lock (&n->note_mu);
	ntime = NOTIFIED_TIME (n);
	if (nsync_time_cmp (ntime, nsync_time_zero) > 0) {
		n->waiters = nsync_dll_remove_ (n->waiters, (nsync_dll_element_ *)nw->q);
		ATM_STORE (&nw->waiting, 0);
	}
	nsync_mu_unlock (&n->note_mu);
	return (ntime);
}

const struct nsync_waitable_funcs_s nsync_note_waitable_funcs = {
	&note_ready_time,
	&note_enqueue,
	&note_dequeue
};

NSYNC_CPP_END_
