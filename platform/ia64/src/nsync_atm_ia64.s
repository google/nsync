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

/* Helper routines for ia64 implementation of atomic operations. */

	.pred.safe_across_calls p1-p5,p16-p63
	.text

/* Atomically, with release barrier semantics:
	int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
		if (*p == old_value) {
			*p = new_value;
			return (some - non - zero - value);
		} else {
			return (0);
		}
	}
 */
	.align 16
	.global nsync_atm_cas_#
	.global nsync_atm_cas_rel_#
	.type	nsync_atm_cas_rel_#, @function
	.proc nsync_atm_cas_rel_#
nsync_atm_cas_:
nsync_atm_cas_rel_:
	.prologue
	.body
	addp4 r33 = r33, r0   /* incoming uint32_t need not have high half zero */
	mov ar.ccv = r33   /* put in ar.ccv for use by cmpxchg */
	cmpxchg4.rel r34 = [r32], r34, ar.ccv
	addl r8 = 1, r0	   /* assume success => return 1 */
	cmp4.eq p6, p7 = r34, r33
	(p7) mov r8 = r0   /* values not equal => return 0 */
	br.ret.sptk.many b0
	.endp nsync_atm_cas_rel_#

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align 16
	.global nsync_atm_cas_acq_#
	.type	nsync_atm_cas_acq_#, @function
	.proc nsync_atm_cas_acq_#
nsync_atm_cas_acq_:
	.prologue
	.body
	addp4 r33 = r33, r0   /* incoming uint32_t need not have high half zero */
	mov ar.ccv = r33   /* put in ar.ccv for use by cmpxchg */
	cmpxchg4.acq r34 = [r32], r34, ar.ccv
	addl r8 = 1, r0	   /* assume success => return 1 */
	cmp4.eq p6, p7 = r34, r33
	(p7) mov r8 = r0   /* values not equal => return 0 */
	br.ret.sptk.many b0
	.endp nsync_atm_cas_acq_#

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align 16
	.global nsync_atm_cas_relacq_#
	.type	nsync_atm_cas_relacq_#, @function
	.proc nsync_atm_cas_relacq_#
nsync_atm_cas_relacq_:
	.prologue
	.body
	addp4 r33 = r33, r0   /* incoming uint32_t need not have high half zero */
	mov ar.ccv = r33   /* put in ar.ccv for use by cmpxchg */
	cmpxchg4.rel r34 = [r32], r34, ar.ccv
	addl r8 = 1, r0	   /* assume success => return 1 */
	mf		   /* acquire barrier */
	cmp4.eq p6, p7 = r34, r33
	(p7) mov r8 = r0   /* values not equal => return 0 */
	br.ret.sptk.many b0
	.endp nsync_atm_cas_relacq_#

/* Atomically:
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align 16
	.global nsync_atm_load_#
	.type	nsync_atm_load_#, @function
	.proc nsync_atm_load_#
nsync_atm_load_:
	.prologue
	.body
	ld4 r8 = [r32]
	br.ret.sptk.many b0
	.endp nsync_atm_load_#

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align 16
	.global nsync_atm_load_acq_#
	.type	nsync_atm_load_acq_#, @function
	.proc nsync_atm_load_acq_#
nsync_atm_load_acq_:
	.prologue
	.body
	ld4.acq r8 = [r32]
	br.ret.sptk.many b0
	.endp nsync_atm_load_acq_#

/* Atomically:
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align 16
	.global nsync_atm_store_#
	.type	nsync_atm_store_#, @function
	.proc nsync_atm_store_#
nsync_atm_store_:
	.prologue
	.body
	st4 [r32] = r33
	br.ret.sptk.many b0
	.endp nsync_atm_store_#

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align 16
	.global nsync_atm_store_rel_#
	.type	nsync_atm_store_rel_#, @function
	.proc nsync_atm_store_rel_#
nsync_atm_store_rel_:
	.prologue
	.body
	st4.rel [r32] = r33
	br.ret.sptk.many b0
	.endp nsync_atm_store_rel_#
