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
#include "sem.h"
#include "dll.h"
#include "common.h"
#include "atomic.h"
#include "time_internal.h"

NSYNC_CPP_START_

int nsync_wait_n (void *mu, void (*lock) (void *), void (*unlock) (void *),
		  nsync_time abs_deadline,
		  int count, struct nsync_waitable_s *waitable[]) {
	int ready;
	IGNORE_RACES_START ();
	for (ready = 0; ready != count &&
			nsync_time_cmp ((*waitable[ready]->funcs->ready_time) (
						waitable[ready]->v, NULL),
					nsync_time_zero) > 0;
	     ready++) {
	}
	if (ready == count) {
		int i;
		int unlocked = 0;
		int j;
		nsync_time ntime;
		waiter *w = nsync_waiter_new_ ();
		struct nsync_waiter_s nw_set[4];
		struct nsync_waiter_s *nw = nw_set;
		ntime = nsync_time_s_ns (1, 1);
		if (count > (int) (sizeof (nw_set) / sizeof (nw_set[0]))) {
			nw = (struct nsync_waiter_s *) malloc (count * sizeof (nw[0]));
		}
		for (i = 0; i != count && nsync_time_cmp (ntime, nsync_time_zero) > 0; i++) {
			nw[i].tag = NSYNC_WAITER_TAG;
			nw[i].sem = &w->sem;
			nsync_dll_init_ ((nsync_dll_element_ *)nw[i].q, &nw[i]);
			ATM_STORE (&nw[i].waiting, 0);
			nw[i].flags = 0;
			ntime = (*waitable[i]->funcs->enqueue) (waitable[i]->v, &nw[i]);
		}

		if (i == count) {
			nsync_time min_ntime;
			nsync_time local_abs_deadline;
			if (mu != NULL) {
				(*unlock) (mu);
				unlocked = 1;
			}
			do {
				min_ntime = nsync_time_no_deadline;
				for (j = 0; j != count; j++) {
					ntime = (*waitable[j]->funcs->ready_time) (
						waitable[j]->v, &nw[j]);
					if (nsync_time_cmp (ntime, min_ntime) < 0) {
						min_ntime = ntime;
					}
				}
				local_abs_deadline = min_ntime;
				if (nsync_time_cmp (abs_deadline, local_abs_deadline) < 0) {
					local_abs_deadline = abs_deadline;
				}
			} while (nsync_time_cmp (min_ntime, nsync_time_zero) > 0 &&
				 nsync_mu_semaphore_p_with_deadline (&w->sem,
					local_abs_deadline) == 0);
		}

		for (j = 0; j != i; j++) {
			ntime = (*waitable[j]->funcs->dequeue) (waitable[j]->v, &nw[j]);
			if (nsync_time_cmp (ntime, nsync_time_zero) == 0 && ready == count) {
				ready = j;
			}
		}

		if (nw != nw_set) {
			free (nw);
		}
		nsync_waiter_free_ (w);
		if (unlocked) {
			(*lock) (mu);
		}
	}
	IGNORE_RACES_END ();
	return (ready);
}

NSYNC_CPP_END_
