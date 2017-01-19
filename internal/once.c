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

static struct once_sync_s {
	nsync_mu once_mu;
	nsync_cv once_cv;
} once_sync[64];

static void nsync_run_once_impl (nsync_once *once, struct once_sync_s *s,
				 void (*f) (void), void (*farg) (void *arg), void *arg) {
	uint32_t o = ATM_LOAD_ACQ (once);
	if (o != 2) {
		unsigned attempts = 0;
		if (s != NULL) {
			nsync_mu_lock (&s->once_mu);
		}
		while (o == 0 && !ATM_CAS_ACQ (once, 0, 1)) {
			o = ATM_LOAD (once);
		}
		if (o == 0) {
			if (s != NULL) {
				nsync_mu_unlock (&s->once_mu);
			}
			if (f != NULL) {
				(*f) ();
			} else {
				(*farg) (arg);
			}
			if (s != NULL) {
				nsync_mu_lock (&s->once_mu);
				nsync_cv_broadcast (&s->once_cv);
			}
			ATM_STORE_REL (once, 2);
		}
		while (ATM_LOAD_ACQ (once) != 2) {
			if (s != NULL) {
				nsync_time deadline;
				if (attempts < 50) {
					attempts += 10;
				}
				deadline = nsync_time_add (nsync_time_now (), nsync_time_ms (attempts));
				nsync_cv_wait_with_deadline (&s->once_cv, &s->once_mu, deadline, NULL);
			} else {
				attempts = nsync_spin_delay_ (attempts);
			}
		}
		if (s != NULL) {
			nsync_mu_unlock (&s->once_mu);
		}
	}
}

void nsync_run_once (nsync_once *once, void (*f) (void)) {
	uint32_t o;
	IGNORE_RACES_START ();
	o = ATM_LOAD_ACQ (once);
	if (o != 2) {
		struct once_sync_s *s = &once_sync[(((uintptr_t) once) / sizeof (*once)) %
						(sizeof (once_sync) / sizeof (once_sync[0]))];
		nsync_run_once_impl (once, s, f, NULL, NULL);
	}
	IGNORE_RACES_END ();
}

void nsync_run_once_arg (nsync_once *once, void (*farg) (void *arg), void *arg) {
	uint32_t o;
	IGNORE_RACES_START ();
	o = ATM_LOAD_ACQ (once);
	if (o != 2) {
		struct once_sync_s *s = &once_sync[(((uintptr_t) once) / sizeof (*once)) %
						(sizeof (once_sync) / sizeof (once_sync[0]))];
		nsync_run_once_impl (once, s, NULL, farg, arg);
	}
	IGNORE_RACES_END ();
}

void nsync_run_once_spin (nsync_once *once, void (*f) (void)) {
	uint32_t o;
	IGNORE_RACES_START ();
	o = ATM_LOAD_ACQ (once);
	if (o != 2) {
		nsync_run_once_impl (once, NULL, f, NULL, NULL);
	}
	IGNORE_RACES_END ();
}

void nsync_run_once_arg_spin (nsync_once *once, void (*farg) (void *arg), void *arg) {
	uint32_t o;
	IGNORE_RACES_START ();
	o = ATM_LOAD_ACQ (once);
	if (o != 2) {
		nsync_run_once_impl (once, NULL, NULL, farg, arg);
	}
	IGNORE_RACES_END ();
}

NSYNC_CPP_END_
