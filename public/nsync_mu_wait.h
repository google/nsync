#ifndef NSYNC_PUBLIC_NSYNC_MU_WAIT_H_
#define NSYNC_PUBLIC_NSYNC_MU_WAIT_H_

#include "nsync_cpp.h"
#include "nsync_mu.h"
#include "nsync_time.h"

NSYNC_CPP_START_

struct nsync_note_s_; /* forward declaration for an nsync_note */

/* Return when (*condition) (condition_arg) is true.  Perhaps unlock and relock *mu while
   blocked waiting for the condition to become true.  nsync_mu_wait() is equivalent
   to nsync_mu_wait_with_deadline() with abs_deadline==nsync_time_no_deadline, and
   cancel_note==NULL.

   Requires that *mu be held on entry.
   Calls *condition only with *mu held, though not always from the
   calling thread, and not always using its lock mode.
   See nsync_mu_wait_with_deadline() for the restrictions on condition and
   performance considerations.  */
void nsync_mu_wait (nsync_mu *mu, int (*condition) (void *condition_arg), void *condition_arg);

/* Return when at least one of:  (*condition) (condition_arg) is true, the
   deadline expires, or *cancel_note is notified.  Perhaps unlock and relock *mu
   while blocked waiting for one of these events, but always return with *mu
   held.  Return 0 iff the (*condition) (condition_arg) is true on return, and
   otherwise either ETIMEDOUT or ECANCELED, depending on why the call returned
   early.  Callers should use abs_deadline==nsync_time_no_deadline for no
   deadline, and cancel_note==NULL for no cancellation.

   Requires that *mu be held on entry.
   Requires that (*condition) (condition_arg) neither modify state protected by
   *mu, nor return a value dependent on state not protected by *mu.  To depend
   on time, use the abs_deadline parameter.
   (Conventional use of condition variables have the same restrictions on the
   conditions tested by the while-loop.)
   The implementation calls *condition only with *mu held, though not
   always from the calling thread, and and not always using its lock mode.

   nsync_mu_wait() and nsync_mu_wait_with_deadline() can be used instead of condition
   variables.  In many straightforward situations they are of equivalent
   performance and are somewhat easier to use, because unlike condition
   variables, they do not require that the waits be placed in a loop, and they
   do not require explicit wakeup calls.  In the current implementation, use of
   nsync_mu_wait() and nsync_mu_wait_with_deadline() on a mutex can take longer if many distinct
   (condition,condition_arg) pairs are used.  In such cases, use an explicit
   condition variable per wakeup condition for best performance.  */
int nsync_mu_wait_with_deadline (nsync_mu *mu,
				 int (*condition) (void *condition_arg), void *condition_arg,
				 nsync_time abs_deadline,
				 struct nsync_note_s_ *cancel_note);

/* Unlock *mu, which must be held in write mode, and wake waiters, if
   appropriate.  Unlike nsync_mu_unlock(), this call is not required to wake
   nsync_mu_wait/nsync_mu_wait_with_deadline calls on conditions that were
   false before this thread acquired the lock.  This call should be used only
   at the end of critical sections for which:
   - nsync_mu_wait and/or nsync_mu_wait_with_deadline are in use on the same
     mutex,
   - this critical section cannot make the condition true for any of those
     nsync_mu_wait/nsync_mu_wait_with_deadline waits, and
   - when performance is significantly improved by doing so. */
void nsync_mu_unlock_without_wakeup (nsync_mu *mu);

NSYNC_CPP_END_

#endif /*NSYNC_PUBLIC_NSYNC_MU_WAIT_H_*/
