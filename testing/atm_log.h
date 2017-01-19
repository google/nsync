#ifndef NSYNC_TESTING_ATM_LOG_H_
#define NSYNC_TESTING_ATM_LOG_H_

#include "platform.h"

void nsync_atm_log_ (int c, void *p, uint32_t o, uint32_t n, const char *file, int line);
void nsync_atm_log_print_ (void);

#endif /*NSYNC_TESTING_ATM_LOG_H_*/
