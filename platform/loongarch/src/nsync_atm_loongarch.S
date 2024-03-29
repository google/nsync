/* Copyright 2023 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

/* Helper routines for loongarch implementation of atomic operations. */

#define zero $r0
#define v0 $r4
#define a0 $r4
#define a1 $r5
#define a2 $r6
#define t0 $r12
#define ra $1

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
	.text
	.align	2
	.globl	nsync_atm_cas_
	.ent	nsync_atm_cas_
nsync_atm_cas_:
1:
	ll.w	t0, a0, 0
	move	v0, zero
	bne 	t0, a1, 2f
	move	v0, a2
	sc.w	v0, a0, 0
	beqz	v0, 1b
	jr  	ra
2:
	jr  	ra
	.end	nsync_atm_cas_

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_acq_
	.ent	nsync_atm_cas_acq_
nsync_atm_cas_acq_:
1:
	ll.w	t0, a0, 0
	move	v0, zero
	bne 	t0, a1, 2f
	move	v0, a2
	sc.w	v0, a0, 0
	beqz 	v0, 1b
	dbar	0
	jr  	ra
2:
	jr  	ra
	.end	nsync_atm_cas_acq_

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_rel_
	.ent	nsync_atm_cas_rel_
nsync_atm_cas_rel_:
	dbar    0
1:
	ll.w	t0, a0, 0
	move	v0, zero
	bne 	t0, a1, 2f
	move	v0, a2
	sc.w	v0, a0, 0
	beqz	v0, 1b
	jr  	ra
2:
	jr  	ra
	.end	nsync_atm_cas_rel_

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_relacq_
	.ent	nsync_atm_cas_relacq_
nsync_atm_cas_relacq_:
	dbar	0
1:
	ll.w	t0, a0, 0
	move	v0, zero
	bne 	t0, a1, 2f
	move	v0, a2
	sc.w	v0, a0, 0
	beqz	v0, 1b
	dbar	0
	jr  	ra
2:
	jr  	ra
	.end	nsync_atm_cas_relacq_

/* Atomically:
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	2
	.globl	nsync_atm_load_
	.ent	nsync_atm_load_
nsync_atm_load_:
	ld.w	v0, a0, 0
	jr  	ra
	.end	nsync_atm_load_

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	2
	.globl	nsync_atm_load_acq_
	.ent	nsync_atm_load_acq_
nsync_atm_load_acq_:
	ld.w    v0, a0, 0
	dbar    0
	jr  	ra
	.end	nsync_atm_load_acq_

/* Atomically:
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	2
	.globl	nsync_atm_store_
	.ent	nsync_atm_store_
nsync_atm_store_:
	st.w	a1, a0, 0
	jr  	ra
	.end	nsync_atm_store_

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	2
	.globl	nsync_atm_store_rel_
	.ent	nsync_atm_store_rel_
nsync_atm_store_rel_:
	dbar	0
	st.w	a1, a0, 0
	jr  	ra
	.end	nsync_atm_store_rel_
