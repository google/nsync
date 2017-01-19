/* Helper routines for arm implementation of atomic operations. */

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
	.align	3
	.global	nsync_atm_cas_
	.type	nsync_atm_cas_, %function
nsync_atm_cas_:
1:
	ldrex	r3, [r0]
	cmp	r3, r1
	bne	2f
	strex	r3, r2, [r0]
	teq	r3, #0
	bne	1b
	mov	r0, #1
	bx	lr
2:
	mov	r0, #0
	bx	lr

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	3
	.global	nsync_atm_cas_acq_
	.type	nsync_atm_cas_acq_, %function
nsync_atm_cas_acq_:
1:
	ldrex	r3, [r0]
	cmp	r3, r1
	bne	2f
	strex	r3, r2, [r0]
	teq	r3, #0
	bne	1b
	dmb	sy
	mov	r0, #1
	bx	lr
2:
	mov	r0, #0
	bx	lr

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	3
	.global	nsync_atm_cas_rel_
	.type	nsync_atm_cas_rel_, %function
nsync_atm_cas_rel_:
	dmb	sy
1:
	ldrex	r3, [r0]
	cmp	r3, r1
	bne	2f
	strex	r3, r2, [r0]
	teq	r3, #0
	bne	1b
	mov	r0, #1
	bx	lr
2:
	mov	r0, #0
	bx	lr

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	3
	.global	nsync_atm_cas_relacq_
	.type	nsync_atm_cas_relacq_, %function
nsync_atm_cas_relacq_:
	dmb	sy
1:
	ldrex	r3, [r0]
	cmp	r3, r1
	bne	2f
	strex	r3, r2, [r0]
	teq	r3, #0
	bne	1b
	dmb	sy
	mov	r0, #1
	bx	lr
2:
	mov	r0, #0
	bx	lr

/* Atomically:
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	3
	.global	nsync_atm_load_
	.type	nsync_atm_load_, %function
nsync_atm_load_:
	ldr	r0, [r0]
	bx	lr

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	3
	.global	nsync_atm_load_acq_
	.type	nsync_atm_load_acq_, %function
nsync_atm_load_acq_:
	ldr	r0, [r0]
	dmb	sy
	bx	lr

/* Atomically:
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	3
	.global	nsync_atm_store_
	.type	nsync_atm_store_, %function
nsync_atm_store_:
	str	r1, [r0]
	bx	lr

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	3
	.global	nsync_atm_store_rel_
	.type	nsync_atm_store_rel_, %function
nsync_atm_store_rel_:
	dmb	sy
	str	r1, [r0]
	bx	lr
