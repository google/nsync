CC=clang++
# Some versions of MacOS (such as Sierra) require _DARWIN_C_SOURCE to include <mutex>, <condition_variable>, and <thread> (!)
PLATFORM_CPPFLAGS=-DNSYNC_USE_CPP11_TIMEPOINT -DNSYNC_ATOMIC_CPP11 -D_DARWIN_C_SOURCE -I../../platform/c++11 -I../../platform/gcc -I../../platform/macos -I../../platform/posix -pthread
PLATFORM_CFLAGS=-x c++ -std=c++11 -Werror -Wall -Wextra -pedantic
PLATFORM_LDFLAGS=-pthread
MKDEP=${CC} -x c++ -M -std=c++11
# Currently, MacOS's C++11 implementation is incomplete, and does not support
# thread_local; hence the use of posix/src/per_thread_waiter.c rather than
# c++11/src/per_thread_waiter.cc
# Similarly, MacOS omits various posix calls, such as clock_gettime().
PLATFORM_C=../../platform/posix/src/clock_gettime.c ../../platform/c++11/src/nsync_semaphore_mutex.cc ../../platform/posix/src/per_thread_waiter.c ../../platform/c++11/src/yield.cc ../../platform/c++11/src/time_rep_timespec.cc ../../platform/c++11/src/nsync_panic.cc
PLATFORM_OBJS=clock_gettime.o nsync_semaphore_mutex.o per_thread_waiter.o yield.o time_rep_timespec.o nsync_panic.o
TEST_PLATFORM_C=../../platform/c++11/src/start_thread.cc
TEST_PLATFORM_OBJS=start_thread.o

include ../../platform/posix/make.common
include dependfile

time_rep_timespec_test.o: ../../platform/c++11/src/time_rep_timespec_test.cc; ${CC} ${CFLAGS} -I../../testing -c ../../platform/c++11/src/time_rep_timespec_test.cc
time_rep_timespec_test: time_rep_timespec_test.o ${TEST_LIB} ${LIB}; ${CC} ${LDFLAGS} -o $@ $@.o ${TEST_LIB} ${LIB} ${PLATFORM_LIBS}
