PLATFORM_CPPFLAGS=-I../../platform/atomic_ind -D_POSIX_C_SOURCE=200809L -I../../platform/decc -I../../platform/osf1 -I../../platform/alpha -I../../platform/posix -pthread -Duintmax_t=uint64_t
PLATFORM_CFLAGS=-std1
PLATFORM_LDFLAGS=-pthread
PLATFORM_LIBS=-lrt
MKDEP=${CC} -M
PLATFORM_C=../../platform/alpha/src/nsync_atm_alpha.s ../../platform/posix/src/nsync_semaphore_mutex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c
PLATFORM_OBJS=nsync_atm_alpha.o nsync_semaphore_mutex.o per_thread_waiter.o yield.o time_rep.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
