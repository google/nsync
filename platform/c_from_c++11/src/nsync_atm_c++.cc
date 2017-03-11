/* Copyright 2016 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

/* Atomic operations on nsync_atomic_uint32_ quantities
   CAS, load, and store.

   Normally, these are used only on nsync_atomic_uint32_ values, but on Linux they may be
   invoked on int values, because futexes operate on int values.  A
   compile-time check in the futex code ensures that both int and   
   nsync_atomic_uint32_ are 32 bits.

   Memory barriers:
	   Operations with the suffixes _ACQ and _RELACQ ensure that the operation
	   appears to complete before other memory operations subsequently performed by
	   the same thread, as seen by other threads.  (In the case of ATM_CAS_ACQ and
	   ATM_CAS_RELACQ, this applies only if the operation returns a non-zero
	   value.)

	   Operations with the suffixes _REL and _RELACQ ensure that the operation
	   appears to complete after other memory operations previously performed by
	   the same thread, as seen by other threads.  (In the case of ATM_CAS_REL and
	   ATM_CAS_RELACQ, this applies only if the operation returns a non-zero value.)

   // Atomically,
   //   int ATM_CAS (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
   //		if (*p == old_value) {
   //			*p = new_value;
   //			return (some-non-zero-value);
   //		} else {
   //			return (0);
   //		}
   //	}
   // *_ACQ, *_REL, and *_RELACQ variants are available,
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

#include <atomic>
#include "platform.h"
#include "nsync_atomic.h"
#include "nsync_cpp.h"

NSYNC_CPP_USING_

NSYNC_C_START_

int assert_atomic32_size_ = 1 / (sizeof (uint32_t) == sizeof (nsync_atomic_uint32_));

typedef std::atomic<uint32_t> nsync_atomic_uint32_cpp_;

// Helper routines for C++ implementation of atomic operations.

int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
	return (std::atomic_compare_exchange_strong_explicit ((nsync_atomic_uint32_cpp_ *) p, &old_value, new_value,
		std::memory_order_relaxed, std::memory_order_relaxed));
}

int nsync_atm_cas_acq_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
	return (std::atomic_compare_exchange_strong_explicit ((nsync_atomic_uint32_cpp_ *) p, &old_value, new_value,
		std::memory_order_acquire, std::memory_order_relaxed));
}

int nsync_atm_cas_rel_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
	return (std::atomic_compare_exchange_strong_explicit ((nsync_atomic_uint32_cpp_ *) p, &old_value, new_value,
		std::memory_order_release, std::memory_order_relaxed));
}

int nsync_atm_cas_relacq_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
	return (std::atomic_compare_exchange_strong_explicit ((nsync_atomic_uint32_cpp_ *) p, &old_value, new_value,
		std::memory_order_acq_rel, std::memory_order_relaxed));
}

uint32_t nsync_atm_load_ (const nsync_atomic_uint32_ *p) {
	return (std::atomic_load_explicit ((nsync_atomic_uint32_cpp_ *) p, std::memory_order_relaxed));
}

uint32_t nsync_atm_load_acq_ (const nsync_atomic_uint32_ *p) {
	return (std::atomic_load_explicit ((nsync_atomic_uint32_cpp_ *) p, std::memory_order_acquire));
}

void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) {
	std::atomic_store_explicit ((nsync_atomic_uint32_cpp_ *) p, value, std::memory_order_relaxed);
}

void nsync_atm_store_rel_ (nsync_atomic_uint32_ *p, uint32_t value) {
	std::atomic_store_explicit ((nsync_atomic_uint32_cpp_ *) p, value, std::memory_order_release);
}

NSYNC_C_END_
