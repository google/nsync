#include "headers.h"

NSYNC_CPP_START_

static pthread_key_t waiter_key;
static nsync_atomic_uint32_ pt_once;

static void do_once (nsync_atomic_uint32_ *ponce, void (*dest) (void *)) {
	uint32_t o = ATM_LOAD_ACQ (ponce);
	if (o != 2) {
		while (o == 0 && !ATM_CAS_ACQ (ponce, 0, 1)) {
			o = ATM_LOAD (ponce);
		}
		if (o == 0) {
			pthread_key_create (&waiter_key, dest);
			ATM_STORE_REL (ponce, 2);
		}
		while (ATM_LOAD_ACQ (ponce) != 2) {
			sched_yield ();
		}
	}
}

void *nsync_per_thread_waiter_ (void (*dest) (void *)) {
	do_once (&pt_once, dest);
	return (pthread_getspecific (waiter_key));
}

void nsync_set_per_thread_waiter_ (void *v, void (*dest) (void *)) {
	do_once (&pt_once, dest);
	pthread_setspecific (waiter_key, v);
}

NSYNC_CPP_END_
