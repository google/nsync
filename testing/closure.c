#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "closure.h"

NSYNC_CPP_START_
void nsync_start_thread_ (void (*f) (void *), void *arg);
NSYNC_CPP_END_
NSYNC_CPP_USING_

/* Run the closure *cl. */
void closure_run (closure *cl) {
	(*cl->f0) (cl);
}

/* Run the closure (closure *), but wrapped to fix the type. */
static void closure_run_body (void *v) {
	closure_run ((closure *)v);
}

void closure_fork (closure *cl) {
	nsync_start_thread_ (&closure_run_body, cl);
}
