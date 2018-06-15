/* Copyright 2018 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

/* Helper routines for riscv implementation of atomic operations. */

	.text

/* Atomically:
        int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
                if (*p == old_value) {
                        *p = new_value;
                        return (some-non-zero-value);
                } else {
                        return (0);
                }
        }
 */
	.align	1
	.globl	nsync_atm_cas_
nsync_atm_cas_:
1:
	lr.w	a5,0(a0)
	bne	a5,a1,2f
	sc.w	a4,a2,0(a0)
	bnez	a4,1b
	li	a0, 1
	ret
2:
	li	a0, 0
	ret

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	1
	.globl	nsync_atm_cas_acq_
nsync_atm_cas_acq_:
1:
	lr.w	a5,0(a0)
	bne	a5,a1,2f
	sc.w.aq	a4,a2,0(a0)
	bnez	a4,1b
	li	a0, 1
	ret
2:
	li	a0, 0
	ret

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	1
	.globl	nsync_atm_cas_rel_
nsync_atm_cas_rel_:
1:
	lr.w.rl	a5,0(a0)
	bne	a5,a1,2f
	sc.w	a4,a2,0(a0)
	bnez	a4,1b
	li	a0, 1
	ret
2:
	li	a0, 0
	ret

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	1
	.globl	nsync_atm_cas_relacq_
nsync_atm_cas_relacq_:
1:
	lr.w.rl	a5,0(a0)
	bne	a5,a1,2f
	sc.w.aq	a4,a2,0(a0)
	bnez	a4,1b
	li	a0, 1
	ret
2:
	li	a0, 0
	ret

/* Atomically:
        uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	1
	.globl	nsync_atm_load_
nsync_atm_load_:
	lw	a0,0(a0)
	ret

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	1
	.globl	nsync_atm_load_acq_
nsync_atm_load_acq_:
	lr.w.aq	a0,0(a0)
	ret

/* Atomically:
        void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	1
	.globl	nsync_atm_store_
nsync_atm_store_:
	amoswap.w zero,a1,0(a0)
	ret

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	1
	.globl	nsync_atm_store_rel_
nsync_atm_store_rel_:
	amoswap.w.rl zero,a1,0(a0)
	ret
