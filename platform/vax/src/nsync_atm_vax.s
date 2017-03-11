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

/* Helper routines for vax implementation of atomic operations. */

/* The vax has no compare-and-swap operation.  Instead, we implement
   interlocked updates to words by protecting such updates with a spin lock.
   This works because on words where nsync uses atomic operations, no
   non-atomic operations are used.  The spin lock is implemented by mapping
   from the word location to a bit in the following array, and using
   test-and-set (bbssi) on that bit.  bbcci is used to clear the bit to
   guarantee isolation between the bits.  */
	.data
	.lcomm  locks,128

	.text

/* Atomically, with acquire and release barrier semantics,
	int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
		if (*p == old_value) {
			*p = new_value;
			return (some-non-zero-value);
		} else {
			return (0);
		}
	}
 */
	.align 1
	.globl nsync_atm_cas_
	.globl nsync_atm_cas_acq_
	.globl nsync_atm_cas_rel_
	.globl nsync_atm_cas_relacq_
nsync_atm_cas_:
nsync_atm_cas_acq_:
nsync_atm_cas_rel_:
nsync_atm_cas_relacq_:
	.word 0x0
1:
	movl 4(%ap),%r0
	ashl $-2,%r0,%r1
	bicl2 $-1024,%r1
	bbssi %r1, locks, 4f
	movl  (%r0),%r2
	clrl  %r3
	cmpl  8(%ap),%r2
	bneq   2f
	movl  12(%ap),(%r0)
	movl  $1, %r3
2:
	bbcci %r1, locks, 3f
3:
	movl  %r3,%r0
	ret
4:
	calls	$0, nsync_yield_
	jmp   1b

/* Atomically, with acquire barrier semantics,
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align 1
	.globl nsync_atm_load_
	.globl nsync_atm_load_acq_
nsync_atm_load_:
nsync_atm_load_acq_:
	.word 0x0
	movl *4(%ap),%r0
	ret

/* Atomically, with release barrier semantics,
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align 1
	.globl nsync_atm_store_
	.globl nsync_atm_store_rel_
nsync_atm_store_:
nsync_atm_store_rel_:
	.word 0x0
1:
	movl 4(%ap),%r0
	ashl $-2,%r0,%r1
	bicl2 $-1024,%r1
	bbssi %r1, locks, 3f
	movl 8(%ap),(%r0)
	bbcci %r1, locks, 2f
2:
	ret
3:
	calls	$0, nsync_yield_
	jmp   1b
