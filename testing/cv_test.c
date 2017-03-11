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

#include "platform.h"
#include "nsync.h"
#include "time_extra.h"
#include "smprintf.h"
#include "testing.h"
#include "closure.h"

NSYNC_CPP_USING_

/* --------------------------- */

/* A cv_queue represents a FIFO queue with up to limit elements.
   The storage for the queue expands as necessary up to limit. */
typedef struct cv_queue_s {
	int limit;          /* max value of count---should not be changed after initialization */
	nsync_cv non_empty; /* signalled when count transitions from zero to non-zero */
	nsync_cv non_full;  /* signalled when count transitions from limit to less than limit */
	nsync_mu mu;        /* protects fields below */
	int pos;            /* index of first in-use element */
	int count;          /* number of elements in use */
	void *data[1];      /* in use elements are data[pos, ..., (pos+count-1)%limit] */
} cv_queue;

/* Return a pointer to new cv_queue. */
static cv_queue *cv_queue_new (int limit) {
	cv_queue *q;
	int size = offsetof (struct cv_queue_s, data) + sizeof (q->data[0]) * limit;
	q = (cv_queue *) malloc (size);
	memset (q, 0, size);
	q->limit = limit;
	return (q);
}

/* Add v to the end of the FIFO *q and return non-zero, or if the FIFO already
   has limit elements and continues to do so until abs_deadline, do nothing and
   return 0. */
static int cv_queue_put (cv_queue *q, void *v, nsync_time abs_deadline) {
	int added = 0;
	int wake = 0;
	nsync_mu_lock (&q->mu);
	while (q->count == q->limit &&
	       nsync_cv_wait_with_deadline (&q->non_full, &q->mu, abs_deadline, NULL) == 0) {
	}
	if (q->count != q->limit) {
		int i = q->pos + q->count;
		if (q->limit <= i) {
			i -= q->limit;
		}
		q->data[i] = v;
		if (q->count == 0) {
			wake = 1;
		}
		q->count++;
		added = 1;
	}
	nsync_mu_unlock (&q->mu);
	if (wake) {
		nsync_cv_broadcast (&q->non_empty);
	}
	return (added);
}

/* Remove the first value from the front of the FIFO *q and return it,
   or if the FIFO is empty and continues to be so until abs_deadline,
   do nothing and return NULL. */
static void *cv_queue_get (cv_queue *q, nsync_time abs_deadline) {
	void *v = NULL;
	nsync_mu_lock (&q->mu);
	while (q->count == 0 &&
	       nsync_cv_wait_with_deadline (&q->non_empty, &q->mu, abs_deadline, NULL) == 0) {
	}
	if (q->count != 0) {
		v = q->data[q->pos];
		q->data[q->pos] = NULL;
		if (q->count == q->limit) {
			nsync_cv_broadcast (&q->non_full);
		}
		q->pos++;
		q->count--;
		if (q->pos == q->limit) {
			q->pos = 0;
		}
	}
	nsync_mu_unlock (&q->mu);
	return (v);
}

/* --------------------------- */

#define INT_TO_PTR(x) ((x) + 1 + (char *)0)
#define PTR_TO_INT(p) (((char *) (p)) - 1 - (char *)0)

/* Put count integers on *q, in the sequence start*3, (start+1)*3, (start+2)*3, .... */
static void producer_cv_n (testing t, cv_queue *q, int start, int count) {
	int i;
	for (i = 0; i != count; i++) {
		if (!cv_queue_put (q, INT_TO_PTR ((start+i)*3), nsync_time_no_deadline)) {
			TEST_FATAL (t, ("cv_queue_put() returned 0 with no deadline"));
		}
	}
}
CLOSURE_DECL_BODY4 (producer_cv_n, testing, cv_queue *, int, int)

/* Get count integers from *q, and check that they are in the
   sequence start*3, (start+1)*3, (start+2)*3, .... */
static void consumer_cv_n (testing t, cv_queue *q, int start, int count) {
	int i;
	for (i = 0; i != count; i++) {
		void *v = cv_queue_get (q, nsync_time_no_deadline);
		int x;
		if (v == NULL) {
			TEST_FATAL (t, ("cv_queue_get() returned NULL with no deadline"));
		}
		x = PTR_TO_INT (v);
		if (x != (start+i)*3) {
			TEST_FATAL (t, ("cv_queue_get() returned bad value; want %d, got %d",
				   (start+i)*3, x));
		}
	}
}

/* CV_PRODUCER_CONSUMER_N is the number of elements passed from producer to consumer in the
   test_cv_producer_consumer*() tests below. */
#define CV_PRODUCER_CONSUMER_N 100000

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**0. */
static void test_cv_producer_consumer0 (testing t) {
	cv_queue *q = cv_queue_new (1);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**1. */
static void test_cv_producer_consumer1 (testing t) {
	cv_queue *q = cv_queue_new (10);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**2. */
static void test_cv_producer_consumer2 (testing t) {
	cv_queue *q = cv_queue_new (100);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**3. */
static void test_cv_producer_consumer3 (testing t) {
	cv_queue *q = cv_queue_new (1000);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**4. */
static void test_cv_producer_consumer4 (testing t) {
	cv_queue *q = cv_queue_new (10 * 1000);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**5. */
static void test_cv_producer_consumer5 (testing t) {
	cv_queue *q = cv_queue_new (100 * 1000);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* Send a stream of integers from a producer thread to
   a consumer thread via a queue with limit 10**6. */
static void test_cv_producer_consumer6 (testing t) {
	cv_queue *q = cv_queue_new (1000 * 1000);
	closure_fork (closure_producer_cv_n (&producer_cv_n, t, q, 0, CV_PRODUCER_CONSUMER_N));
	consumer_cv_n (t, q, 0, CV_PRODUCER_CONSUMER_N);
	free (q);
}

/* The following values control how aggressively we police the timeout. */
#define TOO_EARLY_MS 1
#define TOO_LATE_MS 100   /* longer, to accommodate scheduling delays */
#define TOO_LATE_ALLOWED 25         /* number of iterations permitted to violate too_late */

/* Check timeouts on a CV wait_with_deadline(). */
static void test_cv_deadline (testing t) {
	int too_late_violations;
	nsync_mu mu;
	nsync_cv cv;
	int i;
	nsync_time too_early;
	nsync_time too_late;

	nsync_mu_init (&mu);
	nsync_cv_init (&cv);
	too_early = nsync_time_ms (TOO_EARLY_MS);
	too_late = nsync_time_ms (TOO_LATE_MS);
	too_late_violations = 0;
	nsync_mu_lock (&mu);
	for (i = 0; i != 50; i++) {
		nsync_time end_time;
		nsync_time start_time;
		nsync_time expected_end_time;
		start_time = nsync_time_now ();
		expected_end_time = nsync_time_add (start_time, nsync_time_ms (87));
		if (nsync_cv_wait_with_deadline (&cv, &mu, expected_end_time,
						 NULL) != ETIMEDOUT) {
			TEST_FATAL (t, ("nsync_cv_wait() returned non-expired for a timeout"));
		}
		end_time = nsync_time_now ();
		if (nsync_time_cmp (end_time, nsync_time_sub (expected_end_time, too_early)) < 0) {
			char *elapsed_str = nsync_time_str (nsync_time_sub (expected_end_time, end_time), 2);
			TEST_ERROR (t, ("nsync_cv_wait() returned %s too early", elapsed_str));
			free (elapsed_str);
		}
		if (nsync_time_cmp (nsync_time_add (expected_end_time, too_late), end_time) < 0) {
			too_late_violations++;
		}
	}
	nsync_mu_unlock (&mu);
	if (too_late_violations > TOO_LATE_ALLOWED) {
		TEST_ERROR (t, ("nsync_cv_wait() returned too late %d times", too_late_violations));
	}
}

/* Check cancellations with nsync_cv_wait_with_deadline(). */
static void test_cv_cancel (testing t) {
	nsync_time future_time;
	int too_late_violations;
	nsync_mu mu;
	nsync_cv cv;
	int i;
	nsync_time too_early;
	nsync_time too_late;

	nsync_mu_init (&mu);
	nsync_cv_init (&cv);
	too_early = nsync_time_ms (TOO_EARLY_MS);
	too_late = nsync_time_ms (TOO_LATE_MS);

	/* The loops below cancel after 87 milliseconds, like the timeout tests above. */

	future_time = nsync_time_add (nsync_time_now (), nsync_time_ms (3600000)); /* test cancels with timeout */

	too_late_violations = 0;
	nsync_mu_lock (&mu);
	for (i = 0; i != 50; i++) {
		int x;
		nsync_note cancel;
		nsync_time end_time;
		nsync_time start_time;
		nsync_time expected_end_time;
		start_time = nsync_time_now ();
		expected_end_time = nsync_time_add (start_time, nsync_time_ms (87));

		cancel = nsync_note_new (NULL, expected_end_time);

		x = nsync_cv_wait_with_deadline (&cv, &mu, future_time, cancel);
		if (x != ECANCELED) {
			TEST_FATAL (t, ("nsync_cv_wait() returned non-cancelled (%d) for "
				   "a cancellation; expected %d",
				   x, ECANCELED));
		}
		end_time = nsync_time_now ();
		if (nsync_time_cmp (end_time, nsync_time_sub (expected_end_time, too_early)) < 0) {
			char *elapsed_str = nsync_time_str (nsync_time_sub (expected_end_time, end_time), 2);
			TEST_ERROR (t, ("nsync_cv_wait() returned %s too early", elapsed_str));
			free (elapsed_str);
		}
		if (nsync_time_cmp (nsync_time_add (expected_end_time, too_late), end_time) < 0) {
			too_late_violations++;
		}

		/* Check that an already cancelled wait returns immediately. */
		start_time = nsync_time_now ();

		x = nsync_cv_wait_with_deadline (&cv, &mu, nsync_time_no_deadline, cancel);
		if (x != ECANCELED) {
			TEST_FATAL (t, ("nsync_cv_wait() returned non-cancelled (%d) for "
				   "a cancellation; expected %d",
				   x, ECANCELED));
		}
		end_time = nsync_time_now ();
		if (nsync_time_cmp (end_time, start_time) < 0) {
			char *elapsed_str = nsync_time_str (nsync_time_sub (expected_end_time, end_time), 2);
			TEST_ERROR (t, ("nsync_cv_wait() returned %s too early", elapsed_str));
			free (elapsed_str);
		}
		if (nsync_time_cmp (nsync_time_add (start_time, too_late), end_time) < 0) {
			too_late_violations++;
		}
		nsync_note_notify (cancel);

		nsync_note_free (cancel);
	}
	nsync_mu_unlock (&mu);
	if (too_late_violations > TOO_LATE_ALLOWED) {
		TEST_ERROR (t, ("nsync_cv_wait() returned too late %d times", too_late_violations));
	}
}

/* --------------------------- */

/* Names of debug results for test_cv_debug. */
static const char *result_name[] = {
	"init_mu0",
	"init_cv0",
	"init_mu1",
	"init_cv1",
	"init_mu2",
	"init_cv2",
	"held_mu",
	"wait0_mu",
	"wait0_cv",
	"wait1_mu",
	"wait1_cv",
	"wait2_mu",
	"wait2_cv",
	"wait3_mu",
	"wait3_cv",
	"rheld1_mu",
	"rheld2_mu",
	"rheld1again_mu",
	NULL /* sentinel */
};

/* state for test_cv_debug() */
struct debug_state {
	nsync_mu mu;
	nsync_cv cv;
	int flag;
	char *result[sizeof (result_name) / sizeof (result_name[0])];
};

/* Return a pointer to the slot in s->result[] associated with the
   nul-terminated name[] */
static char **slot (struct debug_state *s, const char *name) {
	int i = 0;
	while (result_name[i] != NULL && strcmp (result_name[i], name) != 0) {
		i++;
	}
	if (result_name[i] == NULL) {  /* caller gave non-existent name */
		abort ();
	}
	return (&s->result[i]);
}

/* Check that the strings associated with nul-terminated strings name0[] and
   name1[] have the same values in s->result[].  */
static void check_same (testing t, struct debug_state *s,
			     const char *name0, const char *name1) {
	if (strcmp (*slot (s, name0), *slot (s, name1)) != 0) {
		TEST_ERROR (t, ("nsync_mu_debug_state() %s state != %s state (%s vs. %s)",
				name0, name1, *slot (s, name0), *slot (s, name1)));
	}
}

/* Check that the strings associated with nul-terminated strings name0[] and
   name1[] have different values in s->result[].  */
static void check_different (testing t, struct debug_state *s,
			     const char *name0, const char *name1) {
	if (strcmp (*slot (s, name0), *slot (s, name1)) == 0) {
		TEST_ERROR (t, ("nsync_mu_debug_state() %s state == %s state",
				name0, name1));
	}
}

/* Return whether the integer at address v is zero. */
static int int_is_zero (const void *v) {
	return (*(int *)v == 0);
}

/* Acquire and release s->mu in write mode, waiting for s->flag==0
   using nsync_mu_wait(). */
static void debug_thread_writer (struct debug_state *s) {
	nsync_mu_lock (&s->mu);
	nsync_mu_wait (&s->mu, &int_is_zero, &s->flag, NULL);
	nsync_mu_unlock (&s->mu);
}

/* Acquire and release s->mu in write mode, waiting for s->flag==0
   using nsync_cv_wait(). */
static void debug_thread_writer_cv (struct debug_state *s) {
	nsync_mu_lock (&s->mu);
	while (s->flag != 0) {
		nsync_cv_wait (&s->cv, &s->mu);
	}
	nsync_mu_unlock (&s->mu);
}

/* Acquire and release s->mu in read mode, waiting for s->flag==0
   using nsync_mu_wait().
   If name!=NULL, record state of s->mu while held using name[]. */
static void debug_thread_reader (struct debug_state *s,
                                 const char *name) {
	nsync_mu_rlock (&s->mu);
	nsync_mu_wait (&s->mu, &int_is_zero, &s->flag, NULL);
	if (name != NULL) {
		int len = 1024;
		*slot (s, name) = nsync_mu_debug_state_and_waiters (
			&s->mu, (char *) malloc (len), len);
	}
	nsync_mu_runlock (&s->mu);
}

/* Acquire and release s->mu in read mode, waiting for s->flag==0
   using nsync_cv_wait().
   If name!=NULL, record state of s->mu while held using name[]. */
static void debug_thread_reader_cv (struct debug_state *s,
                                    const char *name) {
	nsync_mu_rlock (&s->mu);
	while (s->flag != 0) {
		nsync_cv_wait (&s->cv, &s->mu);
	}
	if (name != NULL) {
		int len = 1024;
		*slot (s, name) = nsync_mu_debug_state_and_waiters (
			&s->mu, (char *) malloc (len), len);
	}
	nsync_mu_runlock (&s->mu);
}

CLOSURE_DECL_BODY1 (debug_thread, struct debug_state *)
CLOSURE_DECL_BODY2 (debug_thread_reader, struct debug_state *, const char *)

/* Check that nsync_mu_debug_state() and nsync_cv_debug_state()
   and their variants yield reasonable results.

   The specification of those routines is intentionally loose,
   so this do not check much, but the various possibilities can be 
   examined using the verbose testing flag (-v). */
static void test_cv_debug (testing t) {
	int i;
	int len = 1024;
	char *tmp;
	struct debug_state xs;
	struct debug_state *s = &xs;
	memset (s, 0, sizeof (*s));

	/* Use nsync_*_debugger to check that they work. */
	tmp = nsync_mu_debugger (&s->mu);
	*slot (s, "init_mu0") = strcpy ((char *) malloc (strlen (tmp)+1), tmp);
	tmp = nsync_cv_debugger (&s->cv);
	*slot (s, "init_cv0") = strcpy ((char *) malloc (strlen (tmp)+1), tmp);

	/* Get the same information via the other routines */
	*slot (s, "init_mu1") = nsync_mu_debug_state (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "init_cv1") = nsync_cv_debug_state (
		&s->cv, (char *) malloc (len), len);
	*slot (s, "init_mu2") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "init_cv2") = nsync_cv_debug_state_and_waiters (
		&s->cv, (char *) malloc (len), len);

	nsync_mu_lock (&s->mu);
	*slot (s, "held_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	nsync_mu_unlock (&s->mu);

	/* set up several threads waiting on the mutex */
	nsync_mu_lock (&s->mu);
	s->flag = 1;   /* so thread will block on conditions */
	closure_fork (closure_debug_thread (&debug_thread_writer, s));
	closure_fork (closure_debug_thread (&debug_thread_writer, s));
	closure_fork (closure_debug_thread (&debug_thread_writer, s));
	closure_fork (closure_debug_thread_reader (&debug_thread_reader, s, NULL));
	closure_fork (closure_debug_thread (&debug_thread_writer_cv, s));
	closure_fork (closure_debug_thread (&debug_thread_writer_cv, s));
	closure_fork (closure_debug_thread (&debug_thread_writer_cv, s));
	closure_fork (closure_debug_thread_reader (&debug_thread_reader_cv, s, NULL));
	nsync_time_sleep (nsync_time_ms (500));
	*slot (s, "wait0_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "wait0_cv") = nsync_cv_debug_state_and_waiters (
		&s->cv, (char *) malloc (len), len);

	/* allow the threads to proceed to their conditional waits */
	nsync_mu_unlock (&s->mu);
	nsync_time_sleep (nsync_time_ms (500));
	*slot (s, "wait1_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "wait1_cv") = nsync_cv_debug_state_and_waiters (
		&s->cv, (char *) malloc (len), len);

	nsync_mu_lock (&s->mu);
	/* move cv waiters to mutex queue */
	nsync_cv_broadcast (&s->cv);
	*slot (s, "wait2_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "wait2_cv") = nsync_cv_debug_state_and_waiters (
		&s->cv, (char *) malloc (len), len);

	/* allow all threads to proceed and exit */
	s->flag = 0;
	nsync_mu_unlock (&s->mu);
	nsync_time_sleep (nsync_time_ms (500));
	*slot (s, "wait3_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	*slot (s, "wait3_cv") = nsync_cv_debug_state_and_waiters (
		&s->cv, (char *) malloc (len), len);

	/* Test with more than one reader */
	nsync_mu_rlock (&s->mu);
	*slot (s, "rheld1_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	closure_fork (closure_debug_thread_reader (
		&debug_thread_reader, s, "rheld2_mu"));
	nsync_time_sleep (nsync_time_ms (500));
	*slot (s, "rheld1again_mu") = nsync_mu_debug_state_and_waiters (
		&s->mu, (char *) malloc (len), len);
	nsync_mu_runlock (&s->mu);

	check_same (t, s, "init_mu0", "init_mu1");
	check_same (t, s, "init_mu0", "init_mu2");
	check_same (t, s, "init_cv0", "init_cv1");
	check_same (t, s, "init_cv0", "init_cv2");
	check_different (t, s, "init_mu0", "held_mu");
	check_different (t, s, "rheld1_mu", "held_mu");
	check_different (t, s, "rheld1_mu", "rheld2_mu");
	check_different (t, s, "init_mu0", "init_cv0");

	for (i = 0; result_name[i] != NULL; i++) {
		if (testing_verbose (t)) {
			const char *str = *slot (s, result_name[i]);
			TEST_LOG (t, ("%-16s  %s\n", result_name[i], str));
		}
		if (strlen (s->result[i]) == 0) {
			TEST_ERROR (t, ("nsync_mu_debug_state() %s empty",
					result_name[i]));
		}
		free (s->result[i]);
	}
}

/* --------------------------- */

int main (int argc, char *argv[]) {
	testing_base tb = testing_new (argc, argv, 0);
	TEST_RUN (tb, test_cv_producer_consumer0);
	TEST_RUN (tb, test_cv_producer_consumer1);
	TEST_RUN (tb, test_cv_producer_consumer2);
	TEST_RUN (tb, test_cv_producer_consumer3);
	TEST_RUN (tb, test_cv_producer_consumer4);
	TEST_RUN (tb, test_cv_producer_consumer5);
	TEST_RUN (tb, test_cv_producer_consumer6);
	TEST_RUN (tb, test_cv_deadline);
	TEST_RUN (tb, test_cv_cancel);
	TEST_RUN (tb, test_cv_debug);
	return (testing_base_exit (tb));
}
