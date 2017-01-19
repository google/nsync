#include "platform.h"
#include "array.h"

void a_ensure_ (void *v, int delta, int sz) {
	typedef A_TYPE (void *) a_void_ptr;
	a_void_ptr *a = (a_void_ptr *) v;
	int omax = a->h_.max_;
	if (omax < 0) {
		omax = -omax;
	}
	if (a->h_.len_ + delta > omax) {
		int nmax = a->h_.len_ + delta;
		void *na;
		if (nmax < omax * 2) {
			nmax = omax * 2;
		}
		if (a->h_.max_ <= 0) {
			na = malloc (nmax * sz);
			memcpy (na, a->a_, omax*sz);
		} else {
			na = realloc (a->a_, nmax*sz);
		}
		memset (omax *sz + (char *)na, 0, (nmax - omax) * sz);
		a->a_ = (void **) na;
		a->h_.max_ = nmax;
	}
}
