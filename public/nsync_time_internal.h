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

#ifndef NSYNC_PUBLIC_NSYNC_TIME_INTERNAL_H_
#define NSYNC_PUBLIC_NSYNC_TIME_INTERNAL_H_

#include "nsync_cpp.h"

/* Internal details of the implementation of the type nsync_time.

   The type nsync_time can have different implementations on different
   platforms, because the world has many different representations of time.
   Further, the "epoch" of absolute times can vary from address space to
   address space.

   On monotonic clocks:  In our testing, we found that the monotonic clock on
   various popular systems (such as Linux, and some BSD variants) was no better
   behaved than the realtime clock, and routinely took large steps backwards,
   especially on multiprocessors.  Given that "monotonic" doesn't seem to mean
   what it says, implementers of nsync_time might consider retaining the
   simplicity of a single epoch within an address space, by configuring any
   time synchronization mechanism (like ntp) to adjust for leap seconds by
   adjusting the rate, rather than with a backwards step.  */

#if NSYNC_USE_GPR_TIMESPEC
#include "grpc/support/time.h"
NSYNC_CPP_START_
typedef gpr_timespec nsync_time;
#define NSYNC_TIME_SEC(t) ((t).tv_sec)
#define NSYNC_TIME_NSEC(t) ((t).tv_nsec)
NSYNC_CPP_END_

#elif NSYNC_USE_DEBUG_TIME
/* Check that the library can be built with a different time struct.  */
#include <time.h>
NSYNC_CPP_START_
typedef struct {
	time_t seconds;
	unsigned nanoseconds;
} nsync_time;
#define NSYNC_TIME_SEC(t) ((t).seconds)
#define NSYNC_TIME_NSEC(t) ((t).nanoseconds)
NSYNC_CPP_END_

#else
/* Default is to use timespec. */
#include <time.h>
NSYNC_CPP_START_
typedef struct timespec nsync_time;
#define NSYNC_TIME_SEC(t) ((t).tv_sec)
#define NSYNC_TIME_NSEC(t) ((t).tv_nsec)
NSYNC_CPP_END_

#endif

#endif /*NSYNC_PUBLIC_NSYNC_TIME_INTERNAL_H_*/
