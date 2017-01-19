#ifndef NSYNC_INTERNAL_COMMON_H_
#define NSYNC_INTERNAL_COMMON_H_

#include "nsync_cpp.h"
#include "platform.h"
#include "nsync_atomic.h"
#include "sem.h"
#include "nsync_waiter.h"
#include "dll.h"
#include "nsync_mu.h"
#include "nsync_note.h"

/* Annotations for race detectors. */
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#include "third_party/dynamic_annotations/dynamic_annotations.h"
#define IGNORE_RACES_START() ANNOTATE_IGNORE_READS_AND_WRITES_BEGIN()
#define IGNORE_RACES_END() ANNOTATE_IGNORE_READS_AND_WRITES_END()
#endif
#endif
#if !defined(IGNORE_RACES_START)
#define IGNORE_RACES_START()
#define IGNORE_RACES_END()
#endif

#ifndef NSYNC_DEBUG
#define NSYNC_DEBUG 0
#endif

NSYNC_CPP_START_

/* Yield the CPU. Platform specific. */
void nsync_yield_ (void);

/* Retrieve the per-thread cache of the waiter object.  Platform specific. */
void *nsync_per_thread_waiter_ (void (*dest) (void *));

/* Set the per-thread cache of the waiter object.  Platform specific. */
void nsync_set_per_thread_waiter_ (void *v, void (*dest) (void *));

/* Used in spinloops to delay resumption of the loop.
   Usage:
       unsigned attempts = 0;
       while (try_something) {
	  attempts = nsync_spin_delay_ (attempts);
       } */
unsigned nsync_spin_delay_ (unsigned attempts);

/* Spin until (*w & test) == 0.  It then atomically performs
   *w |= set and returns the previous value of *w.  It performs an acquire
   barrier. */
uint32_t nsync_spin_test_and_set_ (nsync_atomic_uint32_ *w, uint32_t test,
				   uint32_t set, uint32_t clear);

/* Abort after printing the string s. */
void nsync_panic_ (const char *s);

/* ---------- */

#define MIN_(a_,b_) ((a_) < (b_)? (a_) : (b_))
#define MAX_(a_,b_) ((a_) > (b_)? (a_) : (b_))

/* ---------- */

/* Bits in mu.word */
#define MU_WLOCK ((uint32_t) (1 << 0)) /* writer lock is held. */
#define MU_SPINLOCK ((uint32_t) (1 << 1)) /* spinlock is held (protects waiters). */
#define MU_WAITING ((uint32_t) (1 << 2)) /* waiter list is non-empty. */
#define MU_DESIG_WAKER ((uint32_t) (1 << 3)) /* a former waiter awoke, and hasn't yet acquired or slept anew */
#define MU_CONDITION ((uint32_t) (1 << 4)) /* the wait list contains some conditional waiters. */
#define MU_WRITER_WAITING ((uint32_t) (1 << 5)) /* there is a writer waiting */
#define MU_LONG_WAIT ((uint32_t) (1 << 6)) /* the waiter at the head of the queue has been waiting a long time */
#define MU_ALL_FALSE ((uint32_t) (1 << 7)) /* all waiter conditions are false */
#define MU_RLOCK ((uint32_t) (1 << 8)) /* reader count of 1  (count field extends to end of word) */

/* The constants below are derived from those above. */
#define MU_RLOCK_FIELD (~(uint32_t) (MU_RLOCK - 1)) /* mask of reader count field */

#define MU_ANY_LOCK (MU_WLOCK | MU_RLOCK_FIELD) /* mask for any lock held */

#define MU_WZERO_TO_ACQUIRE (MU_ANY_LOCK | MU_LONG_WAIT) /* bits to be zero to acquire write lock */
#define MU_WADD_TO_ACQUIRE (MU_WLOCK)         /* add to acquire a write lock */
#define MU_WHELD_IF_NON_ZERO (MU_WLOCK)       /* if any of these bits are set, write lock is held */
#define MU_WSET_WHEN_WAITING (MU_WAITING | MU_WRITER_WAITING) /* a writer is waiting */
#define MU_WCLEAR_ON_ACQUIRE (MU_WRITER_WAITING)  /* clear MU_WRITER_WAITING is a writer acquires */
#define MU_WCLEAR_ON_UNCONTENDED_RELEASE (MU_ALL_FALSE) /* clear if a writer releases w/o waking */
	
/* bits to be zero to acquire read lock */
#define MU_RZERO_TO_ACQUIRE (MU_WLOCK | MU_WRITER_WAITING | MU_LONG_WAIT)
#define MU_RADD_TO_ACQUIRE (MU_RLOCK)         /* add to acquire a read lock */
#define MU_RHELD_IF_NON_ZERO (MU_RLOCK_FIELD) /* if any of these bits are set, read lock is held */
#define MU_RSET_WHEN_WAITING (MU_WAITING)     /* indicate that some thread is waiting */
#define MU_RCLEAR_ON_ACQUIRE ((uint32_t) 0)              /* nothing to clear when a read acquires */
#define MU_RCLEAR_ON_UNCONTENDED_RELEASE ((uint32_t) 0)  /* nothing to clear when a read releases */
		
	
/* A lock_type holds the values needed to manipulate a mu in some mode (read or
   write).  This allows some of the code to be generic, and parameterized by
   the lock type. */
typedef struct lock_type_s {
	uint32_t zero_to_acquire; /* bits that must be zero to acquire */
	uint32_t add_to_acquire; /* constant to add to acquire */
	uint32_t held_if_non_zero; /* if any of these bits are set, the lock is held */
	uint32_t set_when_waiting; /* set when thread waits */
	uint32_t clear_on_acquire; /* clear when thread acquires */
	uint32_t clear_on_uncontended_release; /* clear when thread releases without waking */
} lock_type;


/* writer_type points to a lock_type that describes how to manipulate a mu for a writer. */
extern lock_type *nsync_writer_type_;

/* reader_type points to a lock_type that describes how to manipulate a mu for a reader. */
extern lock_type *nsync_reader_type_;

/* ---------- */

/* Hold a pair of  condition function and its argument. */
struct wait_condition_s {
	int (*f) (void *v);
	void *v;
};

/* Return whether wait conditions *a_ and *b_ are equal and non-null. */
#define WAIT_CONDITION_EQ(a_, b_)  ((a_)->f != NULL && (a_)->f == (b_)->f && (a_)->v == (b_)->v)

/* If a waiter has waited this many times, it may set the MU_LONG_WAIT bit. */
#define LONG_WAIT_THRESHOLD 30

/* ---------- */

#define NOTIFIED_TIME(n_) (ATM_LOAD_ACQ (&(n_)->notified) != 0? nsync_time_zero : \
			   (n_)->expiry_time_valid? (n_)->expiry_time : nsync_time_no_deadline)

/* A waiter represents a single waiter on a cv or a mu.

   To wait:
   Allocate a waiter struct *w with new_waiter(), set w.waiting=1, and
   w.cv_mu=nil or to the associated mu if waiting on a condition variable, then
   queue w.nsync_dll on some queue, and then wait using:
      while (ATM_LOAD_ACQ (&w.waiting) != 0) { nsync_mu_semaphore_p (&w.sem); }
   Return *w to the freepool by calling free_waiter (w).

   To wakeup:
   Remove *w from the relevant queue then:
    ATM_STORE_REL (&w.waiting, 0);
    nsync_mu_semaphore_v (&w.sem); */
typedef struct {
	uint32_t tag;              /* debug DLL_NSYNC_WAITER, DLL_WAITER, DLL_WAITER_SAMECOND */
	nsync_semaphore sem;       /* Thread waits on this semaphore. */
	struct nsync_waiter_s nw;  /* An embedded nsync_waiter_s. */
	struct nsync_mu_s_ *cv_mu;  /* pointer to nsync_mu associated with a cv wait */
	lock_type *l_type;         /* Lock type of the mu, or nil if not associated with a mu. */
	nsync_atomic_uint32_ remove_count;   /* count of removals from queue */
	struct wait_condition_s cond; /* A condition on which to acquire a mu. */
	nsync_dll_element_ same_condition;   /* Links neighbours in nw.q with same non-nil condition. */
	int flags;                    /* see WAITER_FLAG_* bits */
} waiter;
static const uint32_t WAITER_TAG = 0x0590239fu;
static const uint32_t NSYNC_WAITER_TAG = 0x726d2ba9u;

#define WAITER_RESERVED 0x1  /* waiter reserved by a thread, even when not in use */
#define WAITER_IN_USE   0x2  /* waiter in use by a thread */

#define CONTAINER(t_,f_,p_)  ((t_ *) (((char *) (p_)) - offsetof (t_, f_)))
#define ASSERT(x) do { if (!(x)) { *(volatile int *)0 = 0; } } while (0)
	
/* Return a pointer to the nsync_waiter_s containing nsync_dll_element_ *e. */
#define DLL_NSYNC_WAITER(e) (NSYNC_DEBUG? nsync_dll_nsync_waiter_ (e) : \
	((struct nsync_waiter_s *)((e)->container)))
struct nsync_waiter_s *nsync_dll_nsync_waiter_ (nsync_dll_element_ *e);

/* Return a pointer to the waiter struct that *e is embedded in, where *e is an nw.q field. */
#define DLL_WAITER(e) (NSYNC_DEBUG? nsync_dll_waiter_ (e) : \
	CONTAINER (waiter, nw, DLL_NSYNC_WAITER(e)))
waiter *nsync_dll_waiter_ (nsync_dll_element_ *e);

/* Return a pointer to the waiter struct that *e is embedded in, where *e is a
   same_condition field.  */
#define DLL_WAITER_SAMECOND(e) (NSYNC_DEBUG? nsync_dll_waiter_samecond_ (e) : \
	((waiter *) ((e)->container)))
waiter *nsync_dll_waiter_samecond_ (nsync_dll_element_ *e);

/* Return a pointer to an unused waiter struct.
   Ensures that the enclosed timer is stopped and its channel drained. */
waiter *nsync_waiter_new_ (void);

/* Return an unused waiter struct *w to the free pool. */
void nsync_waiter_free_ (waiter *w);

/* ---------- */

void nsync_mu_lock_slow_ (nsync_mu *mu, waiter *w, uint32_t clear, lock_type *l_type);
void nsync_mu_unlock_slow_ (nsync_mu *mu, lock_type *l_type);
nsync_dll_list_ nsync_remove_from_mu_queue_ (nsync_dll_list_ mu_queue, nsync_dll_element_ *e);
void nsync_maybe_merge_conditions_ (nsync_dll_element_ *p, nsync_dll_element_ *n);
nsync_time nsync_note_notified_deadline_ (nsync_note *n);
int nsync_sem_wait_with_cancel_ (waiter *w, nsync_time abs_deadline,
				 nsync_note *cancel_note);
NSYNC_CPP_END_

#endif /*NSYNC_INTERNAL_COMMON_H_*/
