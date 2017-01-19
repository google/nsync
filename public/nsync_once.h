#ifndef NSYNC_PUBLIC_NSYNC_ONCE_H_
#define NSYNC_PUBLIC_NSYNC_ONCE_H_

#include <inttypes.h>
#include "nsync_cpp.h"
#include "nsync_atomic.h"

NSYNC_CPP_START_

/* An nsync_once allows a function to be called exactly once, when first referenced. */
typedef nsync_atomic_uint32_ nsync_once;

/* An initializer for nsync_once; it is guaranteed to be all zeroes. */
#define NSYNC_ONCE_INIT NSYNC_ATOMIC_UINT32_INIT_

/* The first time nsync_run_once() or nsync_run_once_arg() is applied to *once,
   the supplied function is run (with argument, in the case of nsync_run_once_arg()).
   Other applications will wait until the run of the function is complete, and then
   return without running the function again. */
void nsync_run_once (nsync_once *once, void (*f) (void));
void nsync_run_once_arg (nsync_once *once, void (*farg) (void *arg), void *arg);

/* Same as nsync_run_once()/nsync_run_once_arg() but uses a spinloop.
   Can be used on the same nsync_once as nsync_run_once/nsync_run_once_arg(). */
void nsync_run_once_spin (nsync_once *once, void (*f) (void));
void nsync_run_once_arg_spin (nsync_once *once, void (*farg) (void *arg), void *arg);

NSYNC_CPP_END_

#endif /*NSYNC_PUBLIC_NSYNC_ONCE_H_*/
