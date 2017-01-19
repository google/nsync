#include "platform.h"

NSYNC_CPP_START_

typedef void (*init_once_fn_) (void);
BOOL CALLBACK nsync_init_callback_ (pthread_once_t *o, void *v, void **c) {
        init_once_fn_ *f = (init_once_fn_ *) v;
        (**f) ();
        return (TRUE);
}

NSYNC_CPP_END_
