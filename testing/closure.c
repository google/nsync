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
