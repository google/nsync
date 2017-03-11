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

/* Helper routines for x86_64 implementation of atomic operations. */

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
	.text
	.globl nsync_atm_cas_
	.globl nsync_atm_cas_acq_
	.globl nsync_atm_cas_rel_
	.globl nsync_atm_cas_relacq_
	.type	nsync_atm_cas_, @function
nsync_atm_cas_:
nsync_atm_cas_acq_:
nsync_atm_cas_rel_:
nsync_atm_cas_relacq_:
        movl    %esi, %eax
        lock cmpxchgl   %edx, (%rdi)
        movl    $0, %eax
        sete    %al
        ret

/* Atomically, with acquire barrier semantics,
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.globl nsync_atm_load_
	.globl nsync_atm_load_acq_
	.type	nsync_atm_load_, @function
nsync_atm_load_:
nsync_atm_load_acq_:
	movl    (%rdi), %eax
	ret

/* Atomically, with release barrier semantics,
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.globl nsync_atm_store_
	.globl nsync_atm_store_rel_
	.type	nsync_atm_store_, @function
nsync_atm_store_:
nsync_atm_store_rel_:
	movl	%esi, (%rdi)
	ret
