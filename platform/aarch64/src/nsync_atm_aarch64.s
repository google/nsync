/* Helper routines for aarch64 implementation of atomic operations. */

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
	.align	3
	.global	nsync_atm_cas_
	.type	nsync_atm_cas_, %function
nsync_atm_cas_:
1:
	ldxr	w3, [x0]
	cmp	w3, w1
	bne	2f
	stxr	w4, w2, [x0]
	cbnz	w4, 1b
2:
	cset	w0, eq
	ret

/* Like nsync_atm_cas_, but with acquire barrier semantics. */
	.align	3
	.global	nsync_atm_cas_acq_
	.type	nsync_atm_cas_acq_, %function
nsync_atm_cas_acq_:
1:
	ldaxr	w3, [x0]
	cmp	w3, w1
	bne	2f
	stxr	w4, w2, [x0]
	cbnz	w4, 1b
2:
	cset	w0, eq
	ret

/* Like nsync_atm_cas_, but with release barrier semantics. */
	.align	3
	.global	nsync_atm_cas_rel_
	.type	nsync_atm_cas_rel_, %function
nsync_atm_cas_rel_:
1:
	ldxr	w3, [x0]
	cmp	w3, w1
	bne	2f
	stlxr	w4, w2, [x0]
	cbnz	w4, 1b
2:
	cset	w0, eq
	ret

/* Like nsync_atm_cas_, but with both acquire and release barrier semantics. */
	.align	3
	.global	nsync_atm_cas_relacq_
	.type	nsync_atm_cas_relacq_, %function
nsync_atm_cas_relacq_:
1:
	ldaxr	w3, [x0]
	cmp	w3, w1
	bne	2f
	stlxr	w4, w2, [x0]
	cbnz	w4, 1b
2:
	cset	w0, eq
	ret

/* Atomically:
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align	3
	.global	nsync_atm_load_
	.type	nsync_atm_load_, %function
nsync_atm_load_:
	ldr	w0, [x0]
	ret

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align	3
	.global	nsync_atm_load_acq_
	.type	nsync_atm_load_acq_, %function
nsync_atm_load_acq_:
	ldar	w0, [x0]
	ret

/* Atomically: 
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align	3
	.global	nsync_atm_store_
	.type	nsync_atm_store_, %function
nsync_atm_store_:
	str	w1, [x0]
	ret

/* Like nsync_atm_store_, but with release barrier semantics. */
	.align	3
	.global	nsync_atm_store_rel_
	.type	nsync_atm_store_rel_, %function
nsync_atm_store_rel_:
	stlr	w1, [x0]
	ret
