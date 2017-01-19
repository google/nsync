#include "headers.h"

NSYNC_CPP_START_

struct nsync_thd_args {
	void (*f) (void *);
	void *arg;
};

static DWORD WINAPI body (void *v) {
	struct nsync_thd_args *args = (struct nsync_thd_args *) v;
	(*args->f) (args->arg);
	free (args);
	return (0);
}

void nsync_start_thread_ (void (*f) (void *), void *arg) {
	struct nsync_thd_args *args = (struct nsync_thd_args *) malloc (sizeof (*args));
	HANDLE t;
	args->f = f;
	args->arg = arg;
	t = CreateThread (NULL, 0, &body, args, 0, NULL);
	CloseHandle (t);
}

NSYNC_CPP_END_
