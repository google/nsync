#include "nsync_cpp.h"
#include "platform.h"
#include "compiler.h"
#include "cputype.h"
#include "nsync_time.h"
#include "time_internal.h"

NSYNC_CPP_START_

nsync_time nsync_time_ms (unsigned ms) {
        unsigned s = ms / 1000;
	return (nsync_time_s_ns (s, 1000 * 1000 * (ms - (s * 1000))));  
}

nsync_time nsync_time_us (unsigned us) {
        unsigned s = us / (1000 * 1000);
	return (nsync_time_s_ns (s, 1000 * (us - (s * 1000 * 1000))));
}

NSYNC_CPP_END_
