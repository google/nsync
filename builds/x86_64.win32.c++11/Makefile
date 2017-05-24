PLATFORM_CPPFLAGS=/I../../platform/msvc /I../../platform/c++11  /I../../platform/win32 /I../../platform/posix /DNSYNC_ATOMIC_CPP11
PLATFORM_CFLAGS=/nologo /MT /TP
PLATFORM_CXXFLAGS=/nologo /MT
PLATFORM_LDFLAGS=/nologo /MT

PLATFORM_OBJS=nsync_semaphore_mutex.OBJ yield.OBJ per_thread_waiter.OBJ time_rep_timespec.OBJ nsync_panic.OBJ
TEST_PLATFORM_OBJS=start_thread.OBJ pthread_key_win32.OBJ clock_gettime.OBJ

# ---------------------------------------------

PUBLIC=../../public
INTERNAL=../../internal
TESTING=../../testing

OPT=/std:c++14 /EHsc
CPPFLAGS=$(PLATFORM_CPPFLAGS) /I$(PUBLIC) /I$(INTERNAL)
CFLAGS=$(CPPFLAGS) $(OPT) $(PLATFORM_CFLAGS)
CXXFLAGS=$(CPPFLAGS) $(OPT) $(PLATFORM_CXXFLAGS)
LDFLAGS=$(PLATFORM_LDFLAGS)
PAR_SUB_COUNT=1
PAR_COUNT=2

TESTS=counter_test.EXE cv_mu_timeout_stress_test.EXE cv_test.EXE cv_wait_example_test.EXE dll_test.EXE mu_starvation_test.EXE mu_test.EXE mu_wait_example_test.EXE mu_wait_test.EXE note_test.EXE once_test.EXE pingpong_test.EXE wait_test.EXE

TEST_OBJS=counter_test.OBJ cv_mu_timeout_stress_test.OBJ cv_test.OBJ cv_wait_example_test.OBJ dll_test.OBJ mu_starvation_test.OBJ mu_test.OBJ mu_wait_example_test.OBJ mu_wait_test.OBJ note_test.OBJ once_test.OBJ pingpong_test.OBJ wait_test.OBJ
TEST_LIB_OBJS=array.OBJ atm_log.OBJ closure.OBJ time_extra.OBJ smprintf.OBJ testing.OBJ $(TEST_PLATFORM_OBJS)
LIB_OBJS=common.OBJ counter.OBJ cv.OBJ debug.OBJ dll.OBJ mu.OBJ mu_wait.OBJ note.OBJ time_internal.OBJ once.OBJ sem_wait.OBJ wait.OBJ $(PLATFORM_OBJS)
XLIB=nsync.LIB
TEST_LIB=nsync_test.LIB

all: $(XLIB)

$(XLIB): $(LIB_OBJS)
	lib /nologo /out:$(XLIB) $(LIB_OBJS)

$(TEST_LIB): $(TEST_LIB_OBJS)
	lib /nologo /out:$(TEST_LIB) $(TEST_LIB_OBJS)

clean:
	del $(XLIB) $(TEST_LIB) $(TESTS) $(TEST_OBJS) $(TEST_LIB_OBJS) $(LIB_OBJS) \
	mkdep.OBJ mkdep.EXE run_tests.OBJ run_tests.EXE

tests: $(TESTS)

test: tests run_tests.EXE
	run_tests -b $(TESTS)

partest: tests run_tests.EXE
	run_tests -p $(PAR_COUNT) -n $(PAR_SUB_COUNT) -b $(TESTS)

run_tests.EXE: ../../tools/run_tests.c
	$(CC) /nologo ../../tools/run_tests.c

mkdep.EXE: ../../tools/mkdep.c
	$(CC) /nologo ../../tools/mkdep.c

depend: mkdep.EXE
	mkdep $(CC) /nologo /E $(CPPFLAGS) 2> NUL \
		$(INTERNAL)/common.c $(INTERNAL)/mu.c $(INTERNAL)/sem_wait.c \
		$(INTERNAL)/counter.c $(INTERNAL)/mu_wait.c $(INTERNAL)/sem_wait_no_note.c \
		$(INTERNAL)/cv.c $(INTERNAL)/debug.c $(INTERNAL)/note.c $(INTERNAL)/time_internal.c \
		$(INTERNAL)/dll.c $(INTERNAL)/once.c $(INTERNAL)/wait.c \
		$(TESTING)/array.c $(TESTING)/mu_test.c $(TESTING)/atm_log.c \
		$(TESTING)/mu_wait_example_test.c $(TESTING)/closure.c $(TESTING)/mu_wait_test.c \
		$(TESTING)/counter_test.c $(TESTING)/note_test.c $(TESTING)/once_test.c \
		$(TESTING)/cv_mu_timeout_stress_test.c \
		$(TESTING)/pingpong_test.c $(TESTING)/cv_test.c $(TESTING)/smprintf.c \
		$(TESTING)/cv_wait_example_test.c $(TESTING)/testing.c $(TESTING)/dll_test.c \
		$(TESTING)/time_extra.c $(TESTING)/mu_starvation_test.c $(TESTING)/wait_test.c \
		$(PLATFORM_C) $(PLATFORM_CXX) $(TEST_PLATFORM_C) > dependfile

nsync_semaphore_mutex.OBJ: ../../platform/c++11/src/nsync_semaphore_mutex.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/c++11/src/nsync_semaphore_mutex.cc

yield.OBJ: ../../platform/c++11/src/yield.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/c++11/src/yield.cc

per_thread_waiter.OBJ: ../../platform/c++11/src/per_thread_waiter.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/c++11/src/per_thread_waiter.cc

start_thread.OBJ: ../../platform/c++11/src/start_thread.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/c++11/src/start_thread.cc

clock_gettime.OBJ: ../../platform/win32/src/clock_gettime.c
	$(CC) $(CFLAGS) /c ../../platform/win32/src/clock_gettime.c

time_rep_timespec.OBJ: ../../platform/c++11/src/time_rep_timespec.cc
	$(CC) $(CFLAGS) /c ../../platform/c++11/src/time_rep_timespec.cc

nsync_panic.OBJ: ../../platform/c++11/src/nsync_panic.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/c++11/src/nsync_panic.cc

pthread_key_win32.OBJ: ../../platform/win32/src/pthread_key_win32.cc
	$(CXX) $(CXXFLAGS) /c ../../platform/win32/src/pthread_key_win32.cc

common.OBJ: $(INTERNAL)/common.c; $(CC) $(CFLAGS) /c $(INTERNAL)/common.c
counter.OBJ: $(INTERNAL)/counter.c; $(CC) $(CFLAGS) /c $(INTERNAL)/counter.c
cv.OBJ: $(INTERNAL)/cv.c; $(CC) $(CFLAGS) /c $(INTERNAL)/cv.c
debug.OBJ: $(INTERNAL)/debug.c; $(CC) $(CFLAGS) /c $(INTERNAL)/debug.c
dll.OBJ: $(INTERNAL)/dll.c; $(CC) $(CFLAGS) /c $(INTERNAL)/dll.c
mu.OBJ: $(INTERNAL)/mu.c; $(CC) $(CFLAGS) /c $(INTERNAL)/mu.c
mu_wait.OBJ: $(INTERNAL)/mu_wait.c; $(CC) $(CFLAGS) /c $(INTERNAL)/mu_wait.c
note.OBJ: $(INTERNAL)/note.c; $(CC) $(CFLAGS) /c $(INTERNAL)/note.c
time_internal.OBJ: $(INTERNAL)/time_internal.c; $(CC) $(CFLAGS) /c $(INTERNAL)/time_internal.c
once.OBJ: $(INTERNAL)/once.c; $(CC) $(CFLAGS) /c $(INTERNAL)/once.c
sem_wait.OBJ: $(INTERNAL)/sem_wait.c; $(CC) $(CFLAGS) /c $(INTERNAL)/sem_wait.c
wait.OBJ: $(INTERNAL)/wait.c; $(CC) $(CFLAGS) /c $(INTERNAL)/wait.c

array.OBJ: $(TESTING)/array.c; $(CC) $(CFLAGS) /c $(TESTING)/array.c
atm_log.OBJ: $(TESTING)/atm_log.c; $(CC) $(CFLAGS) /c $(TESTING)/atm_log.c
closure.OBJ: $(TESTING)/closure.c; $(CC) $(CFLAGS) /c $(TESTING)/closure.c
counter_test.OBJ: $(TESTING)/counter_test.c; $(CC) $(CFLAGS) /c $(TESTING)/counter_test.c
cv_mu_timeout_stress_test.OBJ: $(TESTING)/cv_mu_timeout_stress_test.c; $(CC) $(CFLAGS) /c $(TESTING)/cv_mu_timeout_stress_test.c
cv_test.OBJ: $(TESTING)/cv_test.c; $(CC) $(CFLAGS) /c $(TESTING)/cv_test.c
cv_wait_example_test.OBJ: $(TESTING)/cv_wait_example_test.c; $(CC) $(CFLAGS) /c $(TESTING)/cv_wait_example_test.c
dll_test.OBJ: $(TESTING)/dll_test.c; $(CC) $(CFLAGS) /c $(TESTING)/dll_test.c
mu_starvation_test.OBJ: $(TESTING)/mu_starvation_test.c; $(CC) $(CFLAGS) /c $(TESTING)/mu_starvation_test.c
mu_test.OBJ: $(TESTING)/mu_test.c; $(CC) $(CFLAGS) /c $(TESTING)/mu_test.c
mu_wait_example_test.OBJ: $(TESTING)/mu_wait_example_test.c; $(CC) $(CFLAGS) /c $(TESTING)/mu_wait_example_test.c
mu_wait_test.OBJ: $(TESTING)/mu_wait_test.c; $(CC) $(CFLAGS) /c $(TESTING)/mu_wait_test.c
note_test.OBJ: $(TESTING)/note_test.c; $(CC) $(CFLAGS) /c $(TESTING)/note_test.c
once_test.OBJ: $(TESTING)/once_test.c; $(CC) $(CFLAGS) /c $(TESTING)/once_test.c
time_extra.OBJ: $(TESTING)/time_extra.c; $(CC) $(CFLAGS) /c $(TESTING)/time_extra.c
pingpong_test.OBJ: $(TESTING)/pingpong_test.c; $(CC) $(CFLAGS) /c $(TESTING)/pingpong_test.c
smprintf.OBJ: $(TESTING)/smprintf.c; $(CC) $(CFLAGS) /c $(TESTING)/smprintf.c
testing.OBJ: $(TESTING)/testing.c; $(CC) $(CFLAGS) /c $(TESTING)/testing.c
wait_test.OBJ: $(TESTING)/wait_test.c; $(CC) $(CFLAGS) /c $(TESTING)/wait_test.c

counter_test.EXE: counter_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) counter_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
cv_mu_timeout_stress_test.EXE: cv_mu_timeout_stress_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) cv_mu_timeout_stress_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
cv_test.EXE: cv_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) cv_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
cv_wait_example_test.EXE: cv_wait_example_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) cv_wait_example_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
dll_test.EXE: dll_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) dll_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
mu_starvation_test.EXE: mu_starvation_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) mu_starvation_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
mu_test.EXE: mu_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) mu_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
mu_wait_example_test.EXE: mu_wait_example_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) mu_wait_example_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
mu_wait_test.EXE: mu_wait_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) mu_wait_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
note_test.EXE: note_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) note_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
once_test.EXE: once_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) once_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
pingpong_test.EXE: pingpong_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) pingpong_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)
wait_test.EXE: wait_test.OBJ $(TEST_LIB) $(XLIB); $(CC) $(LDFLAGS) wait_test.OBJ $(TEST_LIB) $(XLIB) $(PLATFORM_LIBS)

include dependfile
