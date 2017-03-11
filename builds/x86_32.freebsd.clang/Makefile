PLATFORM_CPPFLAGS=-D_POSIX_C_SOURCE=200809L -I../../platform/clang -I../../platform/freebsd -I../../platform/x86_32 -I../../platform/posix -pthread
PLATFORM_CFLAGS=-Werror -Wall -Wextra -ansi -pedantic -Wno-unneeded-internal-declaration
PLATFORM_LDFLAGS=-pthread
MKDEP=${CC} -M
PLATFORM_C=../../platform/posix/src/nsync_semaphore_mutex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c ../../platform/posix/src/nsync_panic.c
PLATFORM_OBJS=nsync_semaphore_mutex.o per_thread_waiter.o yield.o time_rep.o nsync_panic.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
