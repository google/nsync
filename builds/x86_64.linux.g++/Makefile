PLATFORM_CPPFLAGS=-DNSYNC_ATOMIC_CPP11 -I../../platform/linux -I../../platform/c++11 -D_POSIX_C_SOURCE=200809L -I../../platform/gcc -I../../platform/x86_64 -I../../platform/posix -pthread
PLATFORM_CFLAGS=-x c++ -std=c++11 -Werror -Wall -Wextra -pedantic
PLATFORM_LDFLAGS=-pthread
MKDEP=${CC} -M -x c++ -std=c++11
PLATFORM_C=../../platform/linux/src/nsync_semaphore_futex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c ../../platform/posix/src/nsync_panic.c
PLATFORM_OBJS=nsync_semaphore_futex.o per_thread_waiter.o yield.o time_rep.o nsync_panic.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
