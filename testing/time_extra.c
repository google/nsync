#include "platform.h"
#include "nsync_time.h"
#include "time_internal.h"
#include "time_extra.h"
#include "smprintf.h"
#include "nsync_cpp.h"

NSYNC_CPP_USING_

char *nsync_time_str (nsync_time t, int decimals) {
	static const struct {
		const char *suffix;
		double multiplier;
	} scale[] = {
		{ "ns", 1.0e-9, },
		{ "us", 1e-6, },
		{ "ms", 1e-3, },
		{ "s", 1.0, },
		{ "hr", 3600.0, },
	};
        double s = nsync_time_to_dbl (t);
	int i = 0;
	while (i + 1 != sizeof (scale) / sizeof (scale[0]) && scale[i + 1].multiplier <= s) {
		i++;
	}
	return (smprintf ("%.*f%s", decimals, s/scale[i].multiplier, scale[i].suffix));
}

int nsync_time_sleep_until (nsync_time abs_deadline) {
	int result = 0;
	nsync_time now;
	now = nsync_time_now ();
	if (nsync_time_cmp (abs_deadline, now) > 0) {
		nsync_time remaining;
		remaining = nsync_time_sleep (nsync_time_sub (abs_deadline, now));
		if (nsync_time_cmp (remaining, nsync_time_zero) > 0) {
			result = EINTR;
		}
	}
	return (result);
}

double nsync_time_to_dbl (nsync_time t) {
	return (((double) NSYNC_TIME_SEC (t)) + ((double) NSYNC_TIME_NSEC (t) * 1e-9));
}

nsync_time nsync_time_from_dbl (double d) {
	time_t s = d;
	if (d < s) {
		s--;
	}
	return (nsync_time_s_ns (s, (d - (double) s) * 1e9));
}
