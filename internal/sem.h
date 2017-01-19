#ifndef NSYNC_INTERNAL_SEM_H_
#define NSYNC_INTERNAL_SEM_H_

/* A semaphore.
   It may be counting or binary, and it need have no destructor.  */

#include "nsync_cpp.h"

NSYNC_CPP_START_

typedef struct nsync_semaphore_s_ {
	void *sem[20];
} nsync_semaphore;

/* Initialize *s; the initial value is 0. */
void nsync_mu_semaphore_init (nsync_semaphore *s);

/* Wait until the count of *s exceeds 0, and decrement it. */
void nsync_mu_semaphore_p (nsync_semaphore *s);

/* Wait until one of:
   the count of *s is non-zero, in which case decrement *s and return 0;
   or abs_deadline expires, in which case return ETIMEDOUT. */
int nsync_mu_semaphore_p_with_deadline (nsync_semaphore *s, nsync_time abs_deadline);

/* Ensure that the count of *s is at least 1. */
void nsync_mu_semaphore_v (nsync_semaphore *s);

NSYNC_CPP_END_

#endif /*NSYNC_INTERNAL_SEM_H_*/
