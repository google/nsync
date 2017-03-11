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

/* Helper routines for parisc64 implementation of atomic operations.
   PA RISC uses ldcw (load and clear), so its CAS and store operations
   are interlocked with respect to one another, rather than being fully atomic.
   This is sufficient for nsync.  */

	.LEVEL 2.0
	.text

/* Atomically with respect to other routines in this file, with acquire and
   release semantics:
        int nsync_atm_cas_ (nsync_atomic_uint32_ *p, uint32_t old_value, uint32_t new_value) {
                if (*p == old_value) {
                        *p = new_value;
                        return (some - non - zero - value);
                } else {
                        return (0);
                }
        }
 */
	.align 4
	.globl nsync_atm_cas_
	.globl nsync_atm_cas_acq_
	.globl nsync_atm_cas_rel_
	.globl nsync_atm_cas_relacq_
nsync_atm_cas_:
nsync_atm_cas_acq_:
nsync_atm_cas_rel_:
nsync_atm_cas_relacq_:
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP
	.ENTRY
	stw %r2,-20(%r30)
	ldo 64(%r30),%r30
1:
	/* map input pointer to a 16-byte aligned semaphore */
	ldi 0x70,%r19
	and %r26,%r19,%r19
	addil LR'locks-$global$,%r27
	ldo RR'locks-$global$(%r1),%r1
	addl %r19,%r1,%r23	/* %r23 now points to semaphore */

	ldcw 0(%r23),%r28
	combt,= %r0,%r28,2f
	ldi 1,%r19		/* delay slot */

	ldw,o 0(%r26),%r28
	comclr,<> %r25,%r28,%r28
	ldi 1,%r28		/* nullified if %r25!=previous %r28 */
	comiclr,= 0,%r28,%r0
	stw %r24,0(%r26)	/* nullified if %r28==0 */

	stw,o %r19,0(%r23)

	bv %r0(%r2)
	ldo -64(%r30),%r30	/* delay slot */

2:
	stw %r24,-44(%r30)
	stw %r25,-40(%r30)
	stw %r26,-36(%r30)
	bl nsync_yield_,%r2
	nop			/* delay slot */
	ldw -44(%r30),%r24
	ldw -40(%r30),%r25
	ldw -36(%r30),%r26
	ldw -84(%r30),%r2
	b 1b
	nop			/* delay slot */

	.EXIT
	.PROCEND

/* Atomically with respect to other routines in this file, with release sematics:
        void nsync_atm_store_ (nsync_atomic_uint32_ *p, uint32_t value) { *p = value; } */
	.align 4
	.globl nsync_atm_store_
	.globl nsync_atm_store_rel_
nsync_atm_store_:
nsync_atm_store_rel_:
	.PROC
	.CALLINFO FRAME=64,CALLS,SAVE_RP
	.ENTRY
	stw %r2,-20(%r30)
	ldo 64(%r30),%r30

1:
	/* map input pointer to a 16-byte aligned semaphore */
        ldi 0x70,%r19
        and %r26,%r19,%r19
        addil LR'locks-$global$,%r27
        ldo RR'locks-$global$(%r1),%r1
        addl %r19,%r1,%r23	/* %r23 now points to semaphore */

        ldcw 0(%r23),%r28
        combt,= %r0,%r28,2f
        ldi 1,%r19		/* delay slot */

        stw %r25,0(%r26)

        stw,o %r19,0(%r23)

        bv %r0(%r2)
        ldo -64(%r30),%r30

2:
        stw %r25,-40(%r30)
        stw %r26,-36(%r30)
        bl nsync_yield_,%r2
        nop			/* delay slot */
        ldw -40(%r30),%r25
        ldw -36(%r30),%r26
        ldw -84(%r30),%r2
        b 1b
        nop			/* delay slot */

	.EXIT
	.PROCEND

/* Atomically:
        uint32_t nsync_atm_load_ (nsync_atomic_uint32_ *p) { return (*p); } */
	.align 4
	.globl nsync_atm_load_
nsync_atm_load_:
	.PROC
	.CALLINFO FRAME=0,NO_CALLS
	.ENTRY
	bv %r0(%r2)
	ldw 0(%r26),%r28	/* delay slot */
	.EXIT
	.PROCEND

/* Like nsync_atm_load_, but with acquire barrier semantics. */
	.align 4
	.globl nsync_atm_load_acq_
nsync_atm_load_acq_:
	.PROC
	.CALLINFO FRAME=0,NO_CALLS
	.ENTRY
	bv %r0(%r2)
	ldw,o 0(%r26),%r28	/* delay slot */
	.EXIT
	.PROCEND

	.data
	.align 4
locks:
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
	.word	1
	.word	0
	.word	0
	.word	0
