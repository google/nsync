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

#ifndef NSYNC_TESTING_TIME_EXTRA_H_
#define NSYNC_TESTING_TIME_EXTRA_H_

#include "nsync_cpp.h"

NSYNC_CPP_USING_

/* Return a malloced nul-terminated string representing time t, using
   "decimals" decimal places.  */
char *nsync_time_str (nsync_time t, int decimals);

/* Sleep until the specified time.  Returns 0 on success, and EINTR
   if the call was interrupted. */
int nsync_time_sleep_until (nsync_time abs_deadline);

/* Return t as a double. */
double nsync_time_to_dbl (nsync_time t);

/* Return a time corresponding to double d. */
nsync_time nsync_time_from_dbl (double d);

#endif /*NSYNC_TESTING_TIME_EXTRA_H_*/
