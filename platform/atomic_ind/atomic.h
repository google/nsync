#ifndef NSYNC_PLATFORM_ATOMIC_IND_ATOMIC_H_
#define NSYNC_PLATFORM_ATOMIC_IND_ATOMIC_H_

/* Atomic operations on nsync_atomic_uint32_ quantities
   CAS, load, and store.

   Normally, these are used only on nsync_atomic_uint32_ values, but on Linux they may be
   invoked on int values, because futexes operate on int values.  A
   compile-time check in the futex code ensures that both int and   
   nsync_atomic_uint32_ are 32 bits.

   Memory barriers:
	   Operations with the suffixes _ACQ and _RELACQ ensure that the operation
	   appears to complete before other memory operations subsequently performed by
	   the same thread, as seen by other threads.  (In the case of ATM_CAS_ACQ,
	   this applies only if the operation returns a non-zero value.)

	   Operations with the suffixes _REL and _RELACQ ensure that the operation
	   appears to complete after other memory operations previously performed by
	   the same thread, as seen by other threads.  (In the case of ATM_CAS_REL,
	   this applies only if the operation returns a non-zero value.)

   // Atomically,
   //   int ATM_CAS (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
   //		if (*p == old_value) {
   //			*p = new_value;
   //			return (some-non-zero-value);
   //		} else {
   //			return (0);
   //		}
   //	}
   // *_ACQ, *_REL, *_RELACQ variants are available,
   // with the barrier semantics described above.
   int ATM_CAS (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value);

   // Atomically,
   //     uint32_t ATM_LOAD (nsync_atomic_uint32_ *p) { return (*p); }
   // A *_ACQ variant is available,
   // with the barrier semantics described above.
   uint32_t ATM_LOAD (nsync_atomic_uint32_ *p);

   // Atomically,
   //     void ATM_STORE (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; }
   // A *_REL variant is available,
   // with the barrier semantics described above.
   void ATM_STORE (nsync_atomic_uint32_ *p, uint32_t value);
 */

#include "compiler.h"
#include "nsync_atomic.h"

NSYNC_CPP_START_

int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value);
int nsync_atm_cas_acq_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value);
int nsync_atm_cas_rel_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value);
int nsync_atm_cas_relacq_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value);
uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p);
uint32_t nsync_atm_load_acq_ (nsync_atomic_uint32_ *p);
void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value);
void nsync_atm_store_rel_ (nsync_atomic_uint32_ *p, uint32_t value);

#define ATM_CAS nsync_atm_cas_
#define ATM_CAS_ACQ nsync_atm_cas_acq_
#define ATM_CAS_REL nsync_atm_cas_rel_
#define ATM_CAS_RELACQ nsync_atm_cas_relacq_
#define ATM_LOAD nsync_atm_load_
#define ATM_LOAD_ACQ nsync_atm_load_acq_
#define ATM_STORE nsync_atm_store_
#define ATM_STORE_REL nsync_atm_store_rel_

NSYNC_CPP_END_

#endif /*NSYNC_PLATFORM_ATOMIC_IND_ATOMIC_H_*/
