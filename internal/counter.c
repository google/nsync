#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync.h"
#include "atomic.h"
#include "dll.h"
#include "sem.h"
#include "common.h"
#include "time_internal.h"

NSYNC_CPP_START_

nsync_counter *nsync_counter_init (nsync_counter *c, uint32_t value) {
	IGNORE_RACES_START ();
	memset (c, 0, sizeof (*c));
	ATM_STORE (&c->value, value);
	IGNORE_RACES_END ();
	return (c);
}

uint32_t nsync_counter_add (nsync_counter *c, int32_t delta) {
	uint32_t value;
	IGNORE_RACES_START ();
	if (delta == 0) {
		value = ATM_LOAD_ACQ (&c->value);
	} else {
		nsync_mu_lock (&c->counter_mu);
		do {
			value = ATM_LOAD (&c->value);
		} while (!ATM_CAS_RELACQ (&c->value, value, value+delta));
		value += delta;
		if (delta > 0) {
			ASSERT (value != (uint32_t) delta || !ATM_LOAD (&c->waited));
			ASSERT (value > value - delta);
		} else {
			ASSERT (value < value - delta);
		}
		if (value == 0) {
			nsync_dll_element_ *p;
			while ((p = nsync_dll_first_ (c->waiters)) != NULL) {
				struct nsync_waiter_s *nw = DLL_NSYNC_WAITER (p);
				c->waiters = nsync_dll_remove_ (c->waiters, p);
				ATM_STORE_REL (&nw->waiting, 0);
				nsync_mu_semaphore_v (nw->sem);
			}
		}
		nsync_mu_unlock (&c->counter_mu);
	}
	IGNORE_RACES_END ();
	return (value);
}

uint32_t nsync_counter_value (nsync_counter *c) {
	uint32_t result;
	IGNORE_RACES_START ();
	result = ATM_LOAD_ACQ (&c->value);
	IGNORE_RACES_END ();
	return (result);
}

uint32_t nsync_counter_wait (nsync_counter *c, nsync_time abs_deadline) {
	struct nsync_waitable_s waitable;
	struct nsync_waitable_s *pwaitable = &waitable;
	waitable.v = c;
	waitable.funcs = &nsync_counter_waitable_funcs;
	return (nsync_wait_n (NULL, NULL, NULL, abs_deadline, 1, &pwaitable) == 0? 0:
		ATM_LOAD_ACQ (&c->value));
}

static nsync_time counter_ready_time (void *v, struct nsync_waiter_s *nw UNUSED) {
	nsync_counter *c = (nsync_counter *) v;
	ATM_STORE (&c->waited, 1);
	return (ATM_LOAD_ACQ (&c->value) == 0? nsync_time_zero : nsync_time_no_deadline);
}

static nsync_time counter_enqueue (void *v, struct nsync_waiter_s *nw) {
	nsync_counter *c = (nsync_counter *) v;
	int32_t value;
	nsync_mu_lock (&c->counter_mu);
	value = ATM_LOAD_ACQ (&c->value);
	ATM_STORE (&nw->waiting, 0);
	if (value != 0) {
		c->waiters = nsync_dll_make_last_in_list_ (c->waiters, (nsync_dll_element_ *)nw->q);
		ATM_STORE (&nw->waiting, 1);
	}
	nsync_mu_unlock (&c->counter_mu);
	return (value == 0? nsync_time_zero : nsync_time_no_deadline);
}

static nsync_time counter_dequeue (void *v, struct nsync_waiter_s *nw) {
	nsync_counter *c = (nsync_counter *) v;
	int32_t value;
	nsync_mu_lock (&c->counter_mu);
	value = ATM_LOAD_ACQ (&c->value);
	if (ATM_LOAD_ACQ (&nw->waiting) != 0) {
		c->waiters = nsync_dll_remove_ (c->waiters, (nsync_dll_element_ *)nw->q);
		ATM_STORE (&nw->waiting, 0);
	}
	nsync_mu_unlock (&c->counter_mu);
	return (value == 0? nsync_time_zero : nsync_time_no_deadline);
}

const struct nsync_waitable_funcs_s nsync_counter_waitable_funcs = {
	&counter_ready_time,
	&counter_enqueue,
	&counter_dequeue
};

NSYNC_CPP_END_
