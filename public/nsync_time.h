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

#ifndef NSYNC_PUBLIC_NSYNC_TIME_H_
#define NSYNC_PUBLIC_NSYNC_TIME_H_

#include "nsync_cpp.h"

/* This file is not to be included directly by the client.  It exists because
   different environments insist on different representations of time,
   sometimes because they allow their realtime clocks to go backwards (!).  */

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
struct timespec;
NSYNC_CPP_START_
typedef struct timespec nsync_time;
#define NSYNC_TIME_SEC(t) ((t).tv_sec)
#define NSYNC_TIME_NSEC(t) ((t).tv_nsec)
NSYNC_CPP_END_

#endif

#endif /*NSYNC_PUBLIC_NSYNC_TIME_H_*/
