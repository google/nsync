#include "headers.h"

NSYNC_CPP_START_

struct thd_args {
	void (*f) (void *);
	void *arg;
};

static void *body (void *v) {
	struct thd_args *args = (struct thd_args *) v;
	(*args->f) (args->arg);
	free (args);
	return (NULL);
}

void nsync_start_thread_ (void (*f) (void *), void *arg) {
	struct thd_args *args = (struct thd_args *) malloc (sizeof (*args));
	pthread_t t;
	args->f = f;
	args->arg = arg;
	pthread_create (&t, NULL, body, args);
	pthread_detach (t);
}

NSYNC_CPP_END_
