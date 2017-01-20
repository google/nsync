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

/* Helper routines for alpha implementation of atomic operations. */

#define v0      $0
#define a0      $16
#define a1      $17
#define a2      $18
#define ra      $26

	.set noreorder
	.text

/* Atomically:
	int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
		if (*p == old_value) {
			*p = new_value;
			return (some - non - zero - value);
		} else {
			return (0);
		}
	}
 */
	.align	4
	.globl  nsync_atm_cas_
	.ent 	nsync_atm_cas_
nsync_atm_cas_:
	.frame  $sp, 0, ra
	.prologue 0
1:
	ldl_l	v0, (a0)
	cmpeq	v0, a1, v0
	blbc	v0, 2f
	bis	a2, a2, v0
	stl_c	v0, (a0)
	blbc	v0, 1b
2:
	ret	(ra)
	.end 	nsync_atm_cas_

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	4
	.globl  nsync_atm_cas_acq_
	.ent 	nsync_atm_cas_acq_
nsync_atm_cas_acq_:
	.frame  $sp, 0, ra
	.prologue 0
1:
	ldl_l	v0, (a0)
	cmpeq	v0, a1, v0
	blbc	v0, 2f
	bis	a2, a2, v0
	stl_c	v0, (a0)
	blbc	v0, 1b
	mb
2:
	ret	(ra)
	.end 	nsync_atm_cas_acq_

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	4
	.globl  nsync_atm_cas_rel_
	.ent 	nsync_atm_cas_rel_
nsync_atm_cas_rel_:
	.frame  $sp, 0, ra
	.prologue 0
	mb
1:
	ldl_l	v0, (a0)
	cmpeq	v0, a1, v0
	blbc	v0, 2f
	bis	a2, a2, v0
	stl_c	v0, (a0)
	blbc	v0, 1b
2:
	ret	(ra)
	.end 	nsync_atm_cas_rel_

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	4
	.globl  nsync_atm_cas_relacq_
	.ent 	nsync_atm_cas_relacq_
nsync_atm_cas_relacq_:
	.frame  $sp, 0, ra
	.prologue 0
	mb
1:
	ldl_l	v0, (a0)
	cmpeq	v0, a1, v0
	blbc	v0, 2f
	bis	a2, a2, v0
	stl_c	v0, (a0)
	blbc	v0, 1b
	mb
2:
	ret	(ra)
	.end 	nsync_atm_cas_relacq_

/* Atomically:
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	4
	.globl  nsync_atm_load_
	.ent 	nsync_atm_load_
nsync_atm_load_:
	.frame  $sp, 0, ra
	.prologue 0
	ldl	v0, (a0)
	ret	(ra)
	.end 	nsync_atm_load_

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	4
	.globl  nsync_atm_load_acq_
	.ent 	nsync_atm_load_acq_
nsync_atm_load_acq_:
	.frame  $sp, 0, ra
	.prologue 0
	ldl	v0, (a0)
	mb
	ret	(ra)
	.end 	nsync_atm_load_acq_

/* Atomically:
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	4
	.globl  nsync_atm_store_
	.ent 	nsync_atm_store_
nsync_atm_store_:
	.frame  $sp, 0, ra
	.prologue 0
	stl	a1, (a0)
	ret	(ra)
	.end 	nsync_atm_store_

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	4
	.globl  nsync_atm_store_rel_
	.ent 	nsync_atm_store_rel_
nsync_atm_store_rel_:
	.frame  $sp, 0, ra
	.prologue 0
	mb
	stl	a1, (a0)
	ret	(ra)
	.end 	nsync_atm_store_rel_
