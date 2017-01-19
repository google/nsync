#ifndef NSYNC_PLATFORM_OPENBSD_PLATFORM_H_
#define NSYNC_PLATFORM_OPENBSD_PLATFORM_H_

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE  /* provided ECANCELED */
#endif

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include <stdio.h>
#include <stdarg.h>

#endif /*NSYNC_PLATFORM_OPENBSD_PLATFORM_H_*/
