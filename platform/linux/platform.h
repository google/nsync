#ifndef NSYNC_PLATFORM_LINUX_PLATFORM_H_
#define NSYNC_PLATFORM_LINUX_PLATFORM_H_

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* for futexes */
#endif

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <inttypes.h>
#include <limits.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
#include <stdarg.h>

#endif /*NSYNC_PLATFORM_LINUX_PLATFORM_H_*/
