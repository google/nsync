#ifndef NSYNC_PUBLIC_NSYNC_COUNTER_H_
#define NSYNC_PUBLIC_NSYNC_COUNTER_H_

#include <inttypes.h>
#include "nsync_cpp.h"
#include "nsync_mu.h"
#include "nsync_atomic.h"
#include "nsync_time.h"

NSYNC_CPP_START_

struct nsync_dll_element_s_;

/* An nsync_counter represents an unsigned integer that can count up and down,
   and wake waiters when zero.  */
typedef struct nsync_counter_s_ {
	nsync_atomic_uint32_ waited;    /* wait has been called */
	nsync_mu counter_mu;         /* protects fields below except reads of "value" */
	nsync_atomic_uint32_ value;     /* value of counter */
	struct nsync_dll_element_s_ *waiters;  /* list of waiters */
} nsync_counter;

/* Initializer for a counter with value zero.
   This can be accomplished by zeroing the node. */
#define NSYNC_COUNTER_INIT { NSYNC_ATOMIC_UINT32_INIT_, NSYNC_MU_INIT, \
			     NSYNC_ATOMIC_UINT32_INIT_, NULL }

/* Initialize *c with the given value, and return c. */
nsync_counter *nsync_counter_init (nsync_counter *c, uint32_t value);

/* Add delta to *c, and return its new value.  It is a checked runtime error to
   decrement *c below 0, or to increment *c (i.e., apply a delta > 0) after a
   waiter been woken, and *c has not been initialized anew after the wait.  */
uint32_t nsync_counter_add (nsync_counter *c, int32_t delta);

/* Return the current value of *c. */
uint32_t nsync_counter_value (nsync_counter *c);

/* Wait until *c has value 0, or until abs_deadline, then return
   the value of the *c.  It is a checked runtime error to increment *c after a
   waiter has been woken, and *c has not been initialized anew after the wait.
   */
uint32_t nsync_counter_wait (nsync_counter *c, nsync_time abs_deadline);

NSYNC_CPP_END_

#endif /*NSYNC_PUBLIC_NSYNC_COUNTER_H_*/
