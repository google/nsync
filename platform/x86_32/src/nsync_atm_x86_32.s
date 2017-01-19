/* Helper routines for x86 implementation of atomic operations. */

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
	movl	%esp, %eax
	movl	4(%eax), %edx
	movl	12(%eax), %ecx
	movl	8(%eax), %eax
	lock cmpxchgl	%ecx, (%edx)
	movl	$0, %eax
	sete	%al
	ret

/* Atomically, with acquire barrier semantics,
	uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.globl nsync_atm_load_
	.globl nsync_atm_load_acq_
	.type	nsync_atm_load_, @function
nsync_atm_load_:
nsync_atm_load_acq_:
	movl	%esp, %eax
	movl	4(%eax), %eax
	movl	(%eax), %eax
	ret

/* Atomically, with release barrier semantics,
	void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.globl nsync_atm_store_
	.globl nsync_atm_store_rel_
	.type	nsync_atm_store_, @function
nsync_atm_store_:
nsync_atm_store_rel_:
	movl	%esp, %eax
	movl	8(%eax), %edx
	movl	4(%eax), %eax
	movl	%edx, (%eax)
	ret
