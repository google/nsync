/* Copyright 2018 Google Inc.

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
#include "nsync_time_init.h"
#include "nsync_time.h"
#include "nsync_once.h"

NSYNC_CPP_START_

#define NSYNC_NS_IN_S_ ((nsync_time)(1000 * 1000 * 1000))

/* Return the maximum t, assuming it's an integral
   type, and the representation is not too strange.
   This macro may be used by NSYNC_TIME_MAX_ internally. */
#define MAX_INT_TYPE(t) (((t)~(t)0) > 1?   /*is t unsigned?*/ \
                (t)~(t)0 :  /*unsigned*/ \
                (t) ((((uintmax_t)1) << (sizeof (t) * CHAR_BIT - 1)) - 1)) /*signed*/

const nsync_time nsync_time_no_deadline = NSYNC_TIME_MAX_; /* May use MAX_INT_TYPE() */

const nsync_time nsync_time_zero = 0;

static nsync_once initial_time_once = NSYNC_ONCE_INIT;
struct timespec nsync_time_initial_;
static void get_initial_time (void) {
	clock_gettime (CLOCK_REALTIME, &nsync_time_initial_);
}

nsync_time nsync_time_s_ns (time_t s, unsigned ns) {
	nsync_time result;
	if (sizeof (nsync_time) >= 8) {
		result = ((nsync_time) s) * NSYNC_NS_IN_S_ + (nsync_time) ns;
	} else {
		result = ((nsync_time) s) * (nsync_time) 1000 +
			 ((nsync_time) ns) / (nsync_time) (1000 * 1000);
	}
	return (result);
}

nsync_time nsync_time_now (void) {
	struct timespec ts;
	if (sizeof (nsync_time) < 8) {
		nsync_run_once (&initial_time_once, &get_initial_time);
	}
	clock_gettime (CLOCK_REALTIME, &ts);
	return (nsync_time_s_ns (
			ts.tv_sec - (sizeof (nsync_time) < 8? nsync_time_initial_.tv_sec: 0),
			ts.tv_nsec));
}

nsync_time nsync_time_sleep (nsync_time delay) {
	struct timespec ts;
	struct timespec remain;
	memset (&ts, 0, sizeof (ts));
	ts.tv_sec = NSYNC_TIME_SEC (delay);
	ts.tv_nsec = NSYNC_TIME_NSEC (delay);
	if (nanosleep (&ts, &remain) == 0) {
		/* nanosleep() is not required to fill in "remain"
		   if it returns 0. */
		memset (&remain, 0, sizeof (remain));
	}
	return (nsync_time_s_ns (remain.tv_sec, remain.tv_nsec));
}

nsync_time nsync_time_add (nsync_time a, nsync_time b) {
	return (a+b);
}

nsync_time nsync_time_sub (nsync_time a, nsync_time b) {
	return (a-b);
}

int nsync_time_cmp (nsync_time a, nsync_time b) {
	return ((a > b) - (a < b));
}

NSYNC_CPP_END_
