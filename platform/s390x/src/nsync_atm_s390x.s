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

/* Helper routines for s390x implementation of atomic operations. */

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
	.align	8
	.globl nsync_atm_cas_
	.globl nsync_atm_cas_acq_
        .globl nsync_atm_cas_rel_
        .globl nsync_atm_cas_relacq_
nsync_atm_cas_:
nsync_atm_cas_acq_:
nsync_atm_cas_rel_:
nsync_atm_cas_relacq_:
	/* The compare-and-swap instruction indicates success as an inverted bit
           in the processor status word.   We need to extract it and invert it.
           The sequence is not obvious due to the oddities of the available shift
           and xor instructions. */
	cs	%r3,%r4,0(%r2)  /* CAS old=%3, new=%r4, loc=%r2 ; inverted result->bottom bit of condition code */
	ipm	%r2		/* move condition code to 2**28 bit of %r2, which is return value */
	srl	%r2,28		/* move bit to 2**0 bit of %r2 */
	ahi	%r2,1		/* flip 2**0 bit, so bit has expected sense */
	sll	%r2,31		/* move bit to to 2**31 */
	srl	%r2,31		/* move it back to 2**0 bit, zero exteding */
	llgfr	%r2,%r2		/* zero top 32 bits of %r2 */
	br	%r14		/* return address is %r14 */

/* Atomically, with acquire barrier semantics,
        uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	8
	.globl nsync_atm_load_
	.globl nsync_atm_load_acq_
nsync_atm_load_:
nsync_atm_load_acq_:
	l	%r2,0(%r2)
	llgfr	%r2,%r2
	br	%r14

/* Atomically, with release barrier semantics,
        void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	8
	.globl nsync_atm_store_
	.globl nsync_atm_store_rel_
nsync_atm_store_:
nsync_atm_store_rel_:
	st	%r3,0(%r2)
	br	%r14
