PLATFORM_CPPFLAGS=-I../../platform/atomic_ind -D_POSIX_C_SOURCE=200809L -I../../platform/clang -I../../platform/macos -I../../platform/x86_64 -I../../platform/posix -pthread
PLATFORM_CFLAGS=-Werror -Wall -Wextra -ansi -pedantic -Wno-unneeded-internal-declaration
PLATFORM_LDFLAGS=-pthread
PLATFORM_CXXFLAGS=-Werror -Wall -Wextra -std=c++11 -pedantic
PLATFORM_CXX=../../platform/c_from_c++11/src/nsync_atm_c++.cc
MKDEP_DEPEND=mkdep
MKDEP=./mkdep ${CC} -E -c++=-std=c++11
PLATFORM_C=../../platform/posix/src/clock_gettime.c ../../platform/posix/src/nsync_semaphore_mutex.c ../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c ../../platform/posix/src/time_rep.c ../../platform/posix/src/nsync_panic.c
PLATFORM_OBJS=nsync_atm_c++.o clock_gettime.o nsync_semaphore_mutex.o per_thread_waiter.o yield.o time_rep.o nsync_panic.o
TEST_PLATFORM_C=../../platform/posix/src/start_thread.c
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile
