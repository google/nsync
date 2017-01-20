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

#ifndef NSYNC_PLATFORM_IRIX64_PLATFORM_H_
#define NSYNC_PLATFORM_IRIX64_PLATFORM_H_

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdarg.h>

/* Strangely, some Irix versions have snprintf() and vsnprintf(),
   yet stdio.h lacks valid signatures. */
int snprintf (char *, size_t, const char *, ...);
int vsnprintf (char *, size_t, const char *, va_list);

#endif /*NSYNC_PLATFORM_IRIX64_PLATFORM_H_*/
