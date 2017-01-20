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

#ifndef NSYNC_PUBLIC_NSYNC_NOTE_H_
#define NSYNC_PUBLIC_NSYNC_NOTE_H_

#include <inttypes.h>
#include <time.h>
#include "nsync_cpp.h"
#include "nsync_mu.h"
#include "nsync_cv.h"
#include "nsync_atomic.h"

NSYNC_CPP_START_

struct nsync_dll_element_s_;

/* An nsync_note represents a single bit that can transition from 0 to 1 at
   most once.  When 1, the note is said to be notified.  There are operations
   to wait ofor the transition, which can be triggered either by an explicit
   call, or timer expiry.  Notes can have parent notes; a note becomes notified if its
   parent becomes notified. */
typedef struct nsync_note_s_ {
	void *parent_child_link[3]; /* nsync_dll_element_s_; parent's children, under parent->mu  */
	int expiry_time_valid;      /* whether expiry_time is valid */
	nsync_time expiry_time;     /* expiry time, if expiry_time_valid != 0 */
	nsync_mu note_mu;	   /* protects fields below except "notified" */
	nsync_cv no_children_cv;    /* signalled when children becomes empty */
	uint32_t disconnecting;     /* non-zero => node is being disconnected */
	nsync_atomic_uint32_ notified;	 /* non-zero if the note has been notified */
	struct nsync_note_s_ *parent;	  /* points to parent, if any */
	struct nsync_dll_element_s_ *children; /* list of children */
	struct nsync_dll_element_s_ *waiters;  /* list of waiters */
} nsync_note;

/* Initializer for a note with no parent and no deadline.
   This can be accomplished by zeroing the node. */
#define NSYNC_NOTE_INIT { {NULL, NULL, NULL}, 0 }  /* rely on remaining fields being zeroed */

/* Initialize *n, using *parent as its parent if parent!=NULL, and return n.  A
   note must be notified before deletion if it has waiters, a parent, or
   children.  The note will be automatically notified at abs_deadline,
   and is notified at initialization if abs_deadline==nsync_zero_time.  */
nsync_note *nsync_note_init (nsync_note *n, nsync_note *parent, nsync_time abs_deadline);

/* Notify *n and all its descendents. */
void nsync_note_notify (nsync_note *n);

/* Return whether *n has been notified.  */
int nsync_note_is_notified (nsync_note *n);

/* Wait until *n has been notified or abs_deadline is reached, and return
   whether *n has been notified.  */
int nsync_note_wait (nsync_note *n, nsync_time abs_deadline);

NSYNC_CPP_END_

#endif /*NSYNC_PUBLIC_NSYNC_NOTE_H_*/
