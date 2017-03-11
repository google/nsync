CC=tcc
PLATFORM_CPPFLAGS=-I../../platform/atomic_ind -D_POSIX_C_SOURCE=200809L -I../../platform/tcc -I../../platform/linux -I../../platform/x86_64 -I../../platform/posix -pthread
PLATFORM_CFLAGS=-Werror -Wall
PLATFORM_LDFLAGS=-pthread
MKDEP_DEPEND=mkdep
MKDEP=./mkdep ${CC} -E
PLATFORM_C=../../platform/linux/src/nsync_semaphore_futex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c ../../platform/posix/src/nsync_panic.c
PLATFORM_S=../../platform/x86_64/src/nsync_atm_x86_64.s
PLATFORM_OBJS=nsync_atm_x86_64.o nsync_semaphore_futex.o per_thread_waiter.o yield.o time_rep.o nsync_panic.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
