/* Copyright 2021 Google Inc.

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
	.align	2
	.globl	nsync_atm_cas_
	.type nsync_atm_cas_, @function
nsync_atm_cas_:
1:
	li      $a4,0
	ll.w    $a5,$a0,0
	bne     $a5,$a1,2f
	or      $a4,$a2,$zero
	sc.w    $a4,$a0,0
	beqz    $a4,1b
2:
	or      $a0,$a4,$zero
	jr      $ra

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_acq_
	.type nsync_atm_cas_acq_, @function
nsync_atm_cas_acq_:
1:
	li      $a4,0
	ll.w    $a5,$a0,0
	bne     $a5,$a1,2f
	or      $a4,$a2,$zero
	sc.w    $a4,$a0,0
	beqz    $a4,1b
2:
	dbar    0
	or      $a0,$a4,$zero
	jr      $ra

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_rel_
	.type nsync_atm_cas_rel_, @function
nsync_atm_cas_rel_:
	dbar    0
1:
	li      $a4,0
	ll.w    $a5,$a0,0
	bne     $a5,$a1,2f
	or      $a4,$a2,$zero
	sc.w    $a4,$a0,0
	beqz    $a4,1b
2:
	or      $a0,$a4,$zero
	jr      $ra

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	2
	.globl	nsync_atm_cas_relacq_
	.type nsync_atm_cas_relacq_, @function
nsync_atm_cas_relacq_:
	dbar 0
1:
	li      $a4,0
	ll.w    $a5,$a0,0
	bne     $a5,$a1,2f
	or      $a4,$a2,$zero
	sc.w    $a4,$a0,0
	beqz    $a4,1b
2:
	dbar    0
	or      $a0,$a4,$zero
	jr      $ra

/* Atomically:
        uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	2
	.globl	nsync_atm_load_
	.type nsync_atm_load_, @function
nsync_atm_load_:
	ld.wu   $a0, $a0, 0
	jr      $ra

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	2
	.globl	nsync_atm_load_acq_
	.type nsync_atm_load_acq_, @function
nsync_atm_load_acq_:
	ld.wu   $a0,$a0,0
	dbar 0
	jr      $ra

/* Atomically:
        void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	2
	.globl	nsync_atm_store_
	.type nsync_atm_store_, @function
nsync_atm_store_:
	amswap.w $zero,$a1,$a0
	jr      $ra

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	2
	.globl	nsync_atm_store_rel_
	.type nsync_atm_store_rel_, @function
nsync_atm_store_rel_:
	amswap_db.w $zero,$a1,$a0
	jr      $ra
