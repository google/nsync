#include "platform.h"
#include "smprintf.h"

char *smprintf (const char *fmt, ...) {
	int m = strlen (fmt) * 2 + 1;
	char *buf = (char *) malloc (m);
	int didnt_fit;
	do {
		va_list ap;
		int x;
		va_start (ap, fmt);
		x = vsnprintf (buf, m, fmt, ap);
		va_end (ap);
		if (x >= m) {
			buf = (char *) realloc (buf, m = x+1);
			didnt_fit = 1;
		} else if (x < 0) {
			buf = (char *) realloc (buf, m *= 2);
			didnt_fit = 1;
		} else {
			didnt_fit = 0;
		}
	} while (didnt_fit);
	return (buf);
}
