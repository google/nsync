PLATFORM_CPPFLAGS=-I../../platform/atomic_ind -D_POSIX_C_SOURCE=200809L -I../../platform/gcc_no_tls -I../../platform/gcc -I../../platform/irix64 -I../../platform/mips -I../../platform/posix
PLATFORM_CFLAGS=-Werror -Wall -Wextra -ansi -pedantic
PLATFORM_LIBS=-lpthread
MKDEP=${CC} -M
PLATFORM_C=../../platform/mips/src/nsync_atm_mips.S ../../platform/posix/src/nsync_semaphore_mutex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c ../../platform/posix/src/nsync_panic.c
PLATFORM_OBJS=nsync_atm_mips.o nsync_semaphore_mutex.o per_thread_waiter.o yield.o time_rep.o nsync_panic.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
