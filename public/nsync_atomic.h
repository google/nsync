#ifndef NSYNC_PUBLIC_NSYNC_ATOMIC_H_
#define NSYNC_PUBLIC_NSYNC_ATOMIC_H_

#include "nsync_cpp.h"

/* This file is not to be included directly by the client.  It exists because
   on some platforms, one cannot use a simple uint32_t with atomic operations.
   */
#if NSYNC_ATOMIC_TYPECHECK
#include <inttypes.h>
NSYNC_CPP_START_
typedef struct { uint32_t value; } nsync_atomic_uint32_;
NSYNC_CPP_END_
#define NSYNC_ATOMIC_UINT32_INIT_ { 0 }
#define NSYNC_ATOMIC_UINT32_LOAD_(p) ((p)->value)
#define NSYNC_ATOMIC_UINT32_STORE_(p,v) ((p)->value = (v))
#define NSYNC_ATOMIC_UINT32_PTR_(p) (&(p)->value)

#elif NSYNC_ATOMIC_C11
#include <stdatomic.h>
NSYNC_CPP_START_
typedef uint_least32_t nsync_atomic_uint32_;
NSYNC_CPP_END_
#define NSYNC_ATOMIC_UINT32_INIT_ 0
#define NSYNC_ATOMIC_UINT32_LOAD_(p) (*(p))
#define NSYNC_ATOMIC_UINT32_STORE_(p,v) (*(p) = (v))
#define NSYNC_ATOMIC_UINT32_PTR_(p) (p)

#elif NSYNC_ATOMIC_CPP11
#include <atomic>
NSYNC_CPP_START_
typedef std::atomic<uint32_t> nsync_atomic_uint32_;
NSYNC_CPP_END_
#define NSYNC_ATOMIC_UINT32_INIT_ ATOMIC_VAR_INIT (0)
#define NSYNC_ATOMIC_UINT32_LOAD_(p) (std::atomic_load (p))
#define NSYNC_ATOMIC_UINT32_STORE_(p,v) (std::atomic_store ((p), (uint32_t) (v)))
#define NSYNC_ATOMIC_UINT32_PTR_(p) (p)

#else
#include <inttypes.h>
NSYNC_CPP_START_
typedef uint32_t nsync_atomic_uint32_;
NSYNC_CPP_END_
#define NSYNC_ATOMIC_UINT32_INIT_ 0
#define NSYNC_ATOMIC_UINT32_LOAD_(p) (*(p))
#define NSYNC_ATOMIC_UINT32_STORE_(p,v) (*(p) = (v))
#define NSYNC_ATOMIC_UINT32_PTR_(p) (p)
#endif

#endif /*NSYNC_PUBLIC_NSYNC_ATOMIC_H_*/
