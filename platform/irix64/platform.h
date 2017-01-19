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
