#ifndef NSYNC_PLATFORM_CYGWIN_PLATFORM_H_
#define NSYNC_PLATFORM_CYGWIN_PLATFORM_H_

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <unistd.h>

#endif /*NSYNC_PLATFORM_CYGWIN_PLATFORM_H_*/
