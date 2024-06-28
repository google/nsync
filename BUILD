# -*- mode: python; -*-

# nsync is a C library that exports synchronization primitives, such as reader
# writer locks with conditional critical sections, designed to be open sourced
# in portable C.  See https://github.com/google/nsync
#
# See public/*.h for API.  When compiled with C++11 rather than C, it's in the
# "nsync" name space.
#
# BUILD file usage:
#   deps = "@nsync://nsync" for C version
#   deps = "@nsync://nsync_cpp" for C++11 version.
# The latter uses no OS-specific system calls or architecture-specific atomic
# operations.

load(":bazel/pkg_path_name.bzl", "pkg_path_name")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files([
    "LICENSE",
    "VERSION",
])

# ---------------------------------------------
# Recent versions of bazel have @platforms config_settings for cpu type and
# operating system, replacing its slippery "cpu" value which conflated several
# platform-related concepts.  However, bazel's @platforms still doesn't have
# standardized settings for the compiler type, so config_setting rules
# similar to those below can be found in many packages that use bazel.

config_setting(
    name = "gcc",
    flag_values = {"@bazel_tools//tools/cpp:compiler": "gcc"},
)

config_setting(
    name = "clang",
    flag_values = {"@bazel_tools//tools/cpp:compiler": "clang"},
)

config_setting(
    name = "msvc-cl",
    flag_values = {"@bazel_tools//tools/cpp:compiler": "msvc-cl"},
)

config_setting(
    name = "mingw-gcc",
    flag_values = {"@bazel_tools//tools/cpp:compiler": "mingw-gcc"},
)

config_setting(
    name = "clang-cl",
    flag_values = {"@bazel_tools//tools/cpp:compiler": "clang-cl"},
)

# ---------------------------------------------
# Compilation options.

# Compilation options that apply to both C++11 and C.
NSYNC_OPTS_GENERIC = select({
    # Select the CPU architecture include directory.
    # Some of bazel's CPU specifiers are unclear (e.g., what CPUs are "ppc" vs
    # "ppc32"?) so the correspondences may not be quite right.
    # This select() has no real effect in the C++11 build, but satisfies a
    # #include that would otherwise need a #if.

    # The following ought to work, but lead to errors on some versions of bazel:
    # "@platforms//cpu:aarch32": ["-I" + pkg_path_name() + "/platform/arm"],
    # "@platforms//cpu:armv6-m": ["-I" + pkg_path_name() + "/platform/arm"],
    # "@platforms//cpu:armv7-m": ["-I" + pkg_path_name() + "/platform/arm"],
    # "@platforms//cpu:armv7e-m": ["-I" + pkg_path_name() + "/platform/arm"],
    # "@platforms//cpu:armv7e-mf": ["-I" + pkg_path_name() + "/platform/arm"],
    # "@platforms//cpu:armv8-m": ["-I" + pkg_path_name() + "/platform/aarch64"],
    # "@platforms//cpu:cortex-r52": ["-I" + pkg_path_name() + "/platform/aarch64"],
    # "@platforms//cpu:cortex-r82": ["-I" + pkg_path_name() + "/platform/aarch64"],
    # "@platforms//cpu:ppc32": ["-I" + pkg_path_name() + "/platform/ppc32"],
    # "@platforms//cpu:ppc64le": ["-I" + pkg_path_name() + "/platform/ppc64"],
    # "@platforms//cpu:riscv32": ["-I" + pkg_path_name() + "/platform/riscv"],
    # nsync doesn't yet have a port for these:
    # "@platforms//cpu:wasm32"
    # "@platforms//cpu:wasm64"
    "@platforms//cpu:aarch64": ["-I" + pkg_path_name() + "/platform/aarch64"],
    "@platforms//cpu:arm64_32": ["-I" + pkg_path_name() + "/platform/aarch64"],
    "@platforms//cpu:arm64e": ["-I" + pkg_path_name() + "/platform/aarch64"],
    "@platforms//cpu:armv7": ["-I" + pkg_path_name() + "/platform/arm"],
    "@platforms//cpu:armv7k": ["-I" + pkg_path_name() + "/platform/arm"],
    "@platforms//cpu:i386": ["-I" + pkg_path_name() + "/platform/x86_32"],
    "@platforms//cpu:mips64": ["-I" + pkg_path_name() + "/platform/mips"],
    "@platforms//cpu:ppc": ["-I" + pkg_path_name() + "/platform/ppc32"],
    "@platforms//cpu:riscv64": ["-I" + pkg_path_name() + "/platform/riscv"],
    "@platforms//cpu:s390x": ["-I" + pkg_path_name() + "/platform/s390x"],
    "@platforms//cpu:x86_32": ["-I" + pkg_path_name() + "/platform/x86_32"],
    "@platforms//cpu:x86_64": ["-I" + pkg_path_name() + "/platform/x86_64"],
    "//conditions:default": [],
}) + [
    "-I" + pkg_path_name() + "/public",
    "-I" + pkg_path_name() + "/internal",
    "-I" + pkg_path_name() + "/platform/posix",
] + select({
    "@platforms//os:windows": [],
    "@platforms//os:freebsd": ["-pthread"],
    "//conditions:default": [
        "-D_POSIX_C_SOURCE=200809L",
        "-pthread",
    ],
})

# Options for C build, rather then C++11 build.
NSYNC_OPTS = select({
    # Select the OS include directory.

    # The following ought to work, but lead to errors on some versions of bazel:
    # "@platforms//os:chromiumos": ["-I" + pkg_path_name() + "/platform/linux"],
    # "@platforms//os:netbsd": ["-I" + pkg_path_name() + "/platform/netbsd"],
    # nsync doesn't yet have a port for these:
    # "@platforms//os:emscripten"
    # "@platforms//os:fuchsia"
    # "@platforms//os:haiku"
    # "@platforms//os:nixos"
    # "@platforms//os:qnx"
    # "@platforms//os:tvos"
    # "@platforms//os:visionos"
    # "@platforms//os:vxworks"
    # "@platforms//os:wasi"
    # "@platforms//os:watchos"
    "@platforms//os:android": ["-I" + pkg_path_name() + "/platform/linux"],
    "@platforms//os:freebsd": ["-I" + pkg_path_name() + "/platform/freebsd"],
    "@platforms//os:ios": ["-I" + pkg_path_name() + "/platform/macos"],
    "@platforms//os:linux": ["-I" + pkg_path_name() + "/platform/linux"],
    "@platforms//os:macos": ["-I" + pkg_path_name() + "/platform/macos"],
    "@platforms//os:openbsd": ["-I" + pkg_path_name() + "/platform/openbsd"],
    "@platforms//os:windows": ["-I" + pkg_path_name() + "/platform/win32"],
    "//conditions:default": [],
}) + select({
    # Select the compiler include directory.
    ":gcc": ["-I" + pkg_path_name() + "/platform/gcc"],
    ":clang": ["-I" + pkg_path_name() + "/platform/clang"],
    ":clang-cl": ["-I" + pkg_path_name() + "/platform/clang"],
    ":msvc-cl": ["-I" + pkg_path_name() + "/platform/msvc"],
    "//conditions:default": ["-I" + pkg_path_name() + "/platform/gcc"],
}) + select({
    # Apple deprecated their atomics library, yet recent versions have no
    # working version of stdatomic.h; so some recent versions need one, and
    # other versions prefer the other.  For the moment, just ignore the
    # deprecation.
    "@platforms//os:macos": ["-Wno-deprecated-declarations"],
    "//conditions:default": [],
}) + NSYNC_OPTS_GENERIC

# Options for C++11 build, rather then C build.
NSYNC_OPTS_CPP = select({
    ":msvc-cl": ["/TP"],
    ":clang-cl": ["/TP"],
    "//conditions:default": [
        "-x",
        "c++",
        "-std=c++11",
    ],
}) + select({
    # Some versions of MacOS (notably Sierra) require -D_DARWIN_C_SOURCE
    # to include some standard C++11 headers, like <mutex>.
    "@platforms//os:macos": ["-D_DARWIN_C_SOURCE"],
    "//conditions:default": [],
}) + select({
    # On Linux, the C++11 library's synchronization primitives are
    # surprisingly slow.   See also NSYNC_SRC_PLATFORM_CPP, below.
    "@platforms//os:linux": ["-I" + pkg_path_name() + "/platform/c++11.futex"],
    "//conditions:default": [],
}) + [
    "-DNSYNC_ATOMIC_CPP11",
    "-DNSYNC_USE_CPP11_TIMEPOINT",
    "-I" + pkg_path_name() + "/platform/c++11",
] + select({
    # This section must follow the -I...platform/c++11
    # We assume here that msvc-cl, mingw-gcc, and clang-cl all imply windows.
    # It might have been clearer to use a nested select(), but bazel disallows
    # that.
    "@platforms//os:ios": ["-I" + pkg_path_name() + "/platform/gcc_no_tls"],
    ":msvc-cl": [
        "-I" + pkg_path_name() + "/platform/win32",
        "-I" + pkg_path_name() + "/platform/msvc",
    ],
    ":mingw-gcc": [
        "-I" + pkg_path_name() + "/platform/win32",
        "-I" + pkg_path_name() + "/platform/gcc",
    ],
    ":clang-cl": [
        "-I" + pkg_path_name() + "/platform/win32",
        "-I" + pkg_path_name() + "/platform/clang",
    ],
    ":clang": ["-I" + pkg_path_name() + "/platform/clang"],
    ":gcc": ["-I" + pkg_path_name() + "/platform/gcc"],
    # Hope that anything we do not recognize is gcc-compatible.
    "//conditions:default": ["-I" + pkg_path_name() + "/platform/gcc"],
}) + NSYNC_OPTS_GENERIC

# Link options (for tests) built in C (rather than C++11).
NSYNC_LINK_OPTS = select({
    ":msvc-cl": [],
    ":clang-cl": [],
    "//conditions:default": ["-pthread"],
})

# Link options (for tests) built in C++11 (rather than C).
NSYNC_LINK_OPTS_CPP = select({
    ":msvc-cl": [],
    ":clang-cl": [],
    "//conditions:default": ["-pthread"],
})

# ---------------------------------------------
# Header files the source may include.

# Internal library headers.
NSYNC_INTERNAL_HEADERS = [
    "internal/common.h",
    "internal/dll.h",
    "internal/headers.h",
    "internal/sem.h",
    "internal/wait_internal.h",
]

# Internal test headers.
NSYNC_TEST_HEADERS = NSYNC_INTERNAL_HEADERS + [
    "testing/array.h",
    "testing/atm_log.h",
    "testing/closure.h",
    "testing/heap.h",
    "testing/smprintf.h",
    "testing/testing.h",
    "testing/time_extra.h",
]

# Platform specific headers.
# This declares headers for all platforms, not just the one
# we're building for, to avoid a more complex build file.
NSYNC_INTERNAL_HEADERS_PLATFORM = [
    "platform/aarch64/cputype.h",
    "platform/alpha/cputype.h",
    "platform/arm/cputype.h",
    "platform/atomic_ind/atomic.h",
    "platform/c++11/atomic.h",
    "platform/c++11/platform.h",
    "platform/c++11.futex/platform.h",
    "platform/c11/atomic.h",
    "platform/clang/atomic.h",
    "platform/clang/compiler.h",
    "platform/cygwin/platform.h",
    "platform/decc/compiler.h",
    "platform/freebsd/platform.h",
    "platform/gcc/atomic.h",
    "platform/gcc/compiler.h",
    "platform/gcc_new/atomic.h",
    "platform/gcc_new_debug/atomic.h",
    "platform/gcc_no_tls/compiler.h",
    "platform/gcc_old/atomic.h",
    "platform/lcc/compiler.h",
    "platform/lcc/nsync_time_init.h",
    "platform/linux/platform.h",
    "platform/win32/atomic.h",
    "platform/macos/platform_c++11_os.h",
    "platform/msvc/compiler.h",
    "platform/netbsd/atomic.h",
    "platform/netbsd/platform.h",
    "platform/openbsd/platform.h",
    "platform/osf1/platform.h",
    "platform/macos/atomic.h",
    "platform/macos/platform.h",
    "platform/pmax/cputype.h",
    "platform/posix/cputype.h",
    "platform/posix/nsync_time_init.h",
    "platform/posix/platform_c++11_os.h",
    "platform/ppc32/cputype.h",
    "platform/ppc64/cputype.h",
    "platform/s390x/cputype.h",
    "platform/shark/cputype.h",
    "platform/tcc/compiler.h",
    "platform/win32/platform.h",
    "platform/win32/platform_c++11_os.h",
    "platform/x86_32/cputype.h",
    "platform/x86_64/cputype.h",
]

# ---------------------------------------------
# The nsync library.

# Linux-specific library source.
NSYNC_SRC_LINUX = [
    "platform/linux/src/nsync_semaphore_futex.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# Android-specific library source.
NSYNC_SRC_ANDROID = [
    "platform/posix/src/nsync_semaphore_sem_t.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# MacOS-specific library source.
NSYNC_SRC_MACOS = [
    "platform/posix/src/clock_gettime.c",
    "platform/posix/src/nsync_semaphore_mutex.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# Windows-specific library source.
NSYNC_SRC_WINDOWS = [
    "platform/posix/src/nsync_panic.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/yield.c",
    "platform/win32/src/clock_gettime.c",
    "platform/win32/src/init_callback_win32.c",
    "platform/win32/src/nanosleep.c",
    "platform/win32/src/nsync_semaphore_win32.c",
    "platform/win32/src/pthread_cond_timedwait_win32.c",
    "platform/win32/src/pthread_key_win32.cc",
]

# FreeBSD-specific library source.
NSYNC_SRC_FREEBSD = [
    "platform/posix/src/nsync_semaphore_sem_t.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# NetBSD-specific library source.
NSYNC_SRC_NETBSD = [
    "platform/posix/src/nsync_semaphore_sem_t.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# OpenBSD-specific library source.
NSYNC_SRC_OPENBSD = [
    "platform/posix/src/nsync_semaphore_sem_t.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

# OS-specific library source.
NSYNC_SRC_PLATFORM = select({

    # The following ought to work, but lead to errors on some versions of bazel:
    # "@platforms//os:netbsd": NSYNC_SRC_NETBSD,
    # nsync doesn't yet have a port for these:
    # "@platforms//os:chromiumos"
    # "@platforms//os:emscripten"
    # "@platforms//os:fuchsia"
    # "@platforms//os:haiku"
    # "@platforms//os:nixos"
    # "@platforms//os:qnx"
    # "@platforms//os:tvos"
    # "@platforms//os:visionos"
    # "@platforms//os:vxworks"
    # "@platforms//os:wasi"
    # "@platforms//os:watchos"
    "@platforms//os:android": NSYNC_SRC_ANDROID,
    "@platforms//os:freebsd": NSYNC_SRC_FREEBSD,
    "@platforms//os:ios": NSYNC_SRC_MACOS,
    "@platforms//os:linux": NSYNC_SRC_LINUX,
    "@platforms//os:macos": NSYNC_SRC_MACOS,
    "@platforms//os:openbsd": NSYNC_SRC_OPENBSD,
    "@platforms//os:windows": NSYNC_SRC_WINDOWS,
})

# C++11-specific (OS and architecture independent) library source.
NSYNC_SRC_PLATFORM_CPP = [
    "platform/c++11/src/time_rep_timespec.cc",
    "platform/c++11/src/nsync_panic.cc",
    "platform/c++11/src/yield.cc",
] + select({
    # On Linux, the C++11 library's synchronization primitives are surprisingly
    # slow, at least at the time of writing (early 2018).  Raw kernel
    # primitives are ten times faster for wakeups.
    "@platforms//os:linux": ["platform/linux/src/nsync_semaphore_futex.c"],
    "//conditions:default": ["platform/c++11/src/nsync_semaphore_mutex.cc"],
}) + select({
    # MacOS and Android don't have working C++11 thread local storage.
    "@platforms//os:macos": ["platform/posix/src/per_thread_waiter.c"],
    "@platforms//os:android": ["platform/posix/src/per_thread_waiter.c"],
    "@platforms//os:ios": ["platform/posix/src/per_thread_waiter.c"],
    "@platforms//os:windows": [
        "platform/win32/src/clock_gettime.c",
        # Windows has no thread-specific data with thread-exit destructors; we
        # must emulate it with C++ per-thread class destructors.
        "platform/win32/src/pthread_key_win32.cc",
        "platform/win32/src/per_thread_waiter.c",
    ],
    # It's dangerous to use C++ class destructors if we can avoid it, because
    # nsync may be linked into the address space multiple times.
    "//conditions:default": ["platform/posix/src/per_thread_waiter.c"],
})

# Generic library source.
NSYNC_SRC_GENERIC = [
    "internal/common.c",
    "internal/counter.c",
    "internal/cv.c",
    "internal/debug.c",
    "internal/dll.c",
    "internal/mu.c",
    "internal/mu_wait.c",
    "internal/note.c",
    "internal/once.c",
    "internal/sem_wait.c",
    "internal/time_internal.c",
    "internal/wait.c",
]

# Generic library header files.
NSYNC_HDR_GENERIC = [
    "public/nsync.h",
    "public/nsync_atomic.h",
    "public/nsync_counter.h",
    "public/nsync_cpp.h",
    "public/nsync_cv.h",
    "public/nsync_debug.h",
    "public/nsync_mu.h",
    "public/nsync_mu_wait.h",
    "public/nsync_note.h",
    "public/nsync_once.h",
    "public/nsync_time.h",
    "public/nsync_time_internal.h",
    "public/nsync_waiter.h",
]

# The library compiled in C, rather than C++11.
cc_library(
    name = "nsync",
    srcs = NSYNC_SRC_GENERIC + NSYNC_SRC_PLATFORM,
    hdrs = NSYNC_HDR_GENERIC,
    copts = NSYNC_OPTS,
    includes = ["public"],
    textual_hdrs = NSYNC_INTERNAL_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
)

# The library compiled in C++11, rather than C.
cc_library(
    name = "nsync_cpp",
    srcs = NSYNC_SRC_GENERIC + NSYNC_SRC_PLATFORM_CPP,
    hdrs = NSYNC_HDR_GENERIC,
    copts = NSYNC_OPTS_CPP,
    includes = ["public"],
    textual_hdrs = NSYNC_INTERNAL_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
)

# nsync_headers provides just the header files for use in projects that need to
# build shared libraries for dynamic loading.  Bazel seems unable to cope
# otherwise.
cc_library(
    name = "nsync_headers",
    hdrs = glob(["public/*.h"]),
    includes = ["public"],
)

# ---------------------------------------------
# Test code.

# Linux-specific test library source.
NSYNC_TEST_SRC_LINUX = [
    "platform/posix/src/start_thread.c",
]

# Android-specific test library source.
NSYNC_TEST_SRC_ANDROID = [
    "platform/posix/src/start_thread.c",
]

# MacOS-specific test library source.
NSYNC_TEST_SRC_MACOS = [
    "platform/posix/src/start_thread.c",
]

# Windows-specific test library source.
NSYNC_TEST_SRC_WINDOWS = [
    "platform/win32/src/start_thread.c",
]

# FreeBSD-specific test library source.
NSYNC_TEST_SRC_FREEBSD = [
    "platform/posix/src/start_thread.c",
]

# NetBSD-specific test library source.
NSYNC_TEST_SRC_NETBSD = [
    "platform/posix/src/start_thread.c",
]

# OpenBSD-specific test library source.
NSYNC_TEST_SRC_OPENBSD = [
    "platform/posix/src/start_thread.c",
]

# POSIX test library source.
NSYNC_TEST_SRC_POSIX = [
    "platform/posix/src/start_thread.c",
]

# OS-specific test library source.
NSYNC_TEST_SRC_PLATFORM = select({
    # Select the OS include directory.

    # The following ought to work, but lead to errors on some versions of bazel:
    # "@platforms//os:netbsd": NSYNC_TEST_SRC_NETBSD,
    # nsync doesn't yet have a port for these:
    # "@platforms//os:chromiumos"
    # "@platforms//os:emscripten"
    # "@platforms//os:fuchsia"
    # "@platforms//os:haiku"
    # "@platforms//os:nixos"
    # "@platforms//os:qnx"
    # "@platforms//os:tvos"
    # "@platforms//os:visionos"
    # "@platforms//os:vxworks"
    # "@platforms//os:wasi"
    # "@platforms//os:watchos"
    "@platforms//os:android": NSYNC_TEST_SRC_ANDROID,
    "@platforms//os:freebsd": NSYNC_TEST_SRC_FREEBSD,
    "@platforms//os:ios": NSYNC_TEST_SRC_MACOS,
    "@platforms//os:linux": NSYNC_TEST_SRC_LINUX,
    "@platforms//os:macos": NSYNC_TEST_SRC_MACOS,
    "@platforms//os:openbsd": NSYNC_TEST_SRC_OPENBSD,
    "@platforms//os:windows": NSYNC_TEST_SRC_WINDOWS,
    "//conditions:default": NSYNC_TEST_SRC_POSIX,
})

# C++11-specific (OS and architecture independent) test library source.
NSYNC_TEST_SRC_PLATFORM_CPP = [
    "platform/c++11/src/start_thread.cc",
]

# Generic test library source.
NSYNC_TEST_SRC_GENERIC = [
    "testing/array.c",
    "testing/atm_log.c",
    "testing/closure.c",
    "testing/smprintf.c",
    "testing/testing.c",
    "testing/time_extra.c",
]

# The test library compiled in C, rather than C++11.
cc_library(
    name = "nsync_test_lib",
    testonly = 1,
    srcs = NSYNC_TEST_SRC_GENERIC + NSYNC_TEST_SRC_PLATFORM,
    hdrs = ["testing/testing.h"],
    copts = NSYNC_OPTS,
    textual_hdrs = NSYNC_TEST_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
    deps = [":nsync"],
)

# The test library compiled in C++11, rather than C.
cc_library(
    name = "nsync_test_lib_cpp",
    testonly = 1,
    srcs = NSYNC_TEST_SRC_GENERIC + NSYNC_TEST_SRC_PLATFORM_CPP,
    hdrs = ["testing/testing.h"],
    copts = NSYNC_OPTS_CPP,
    textual_hdrs = NSYNC_TEST_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
    deps = [":nsync_cpp"],
)

# ---------------------------------------------
# The tests, compiled in C rather than C++11.

cc_test(
    name = "counter_test",
    size = "small",
    srcs = ["testing/counter_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "cv_mu_timeout_stress_test",
    size = "small",
    srcs = ["testing/cv_mu_timeout_stress_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "cv_test",
    size = "small",
    srcs = ["testing/cv_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "cv_wait_example_test",
    size = "small",
    srcs = ["testing/cv_wait_example_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "dll_test",
    size = "small",
    srcs = ["testing/dll_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "mu_starvation_test",
    size = "small",
    srcs = ["testing/mu_starvation_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "mu_test",
    size = "small",
    srcs = ["testing/mu_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "mu_wait_example_test",
    size = "small",
    srcs = ["testing/mu_wait_example_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "mu_wait_test",
    size = "small",
    srcs = ["testing/mu_wait_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "note_test",
    size = "small",
    srcs = ["testing/note_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "once_test",
    size = "small",
    srcs = ["testing/once_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "pingpong_test",
    size = "small",
    srcs = ["testing/pingpong_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

cc_test(
    name = "wait_test",
    size = "small",
    srcs = ["testing/wait_test.c"],
    copts = NSYNC_OPTS,
    linkopts = NSYNC_LINK_OPTS,
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)

# ---------------------------------------------
# The tests, compiled in C++11, rather than C.

cc_test(
    name = "counter_cpp_test",
    size = "small",
    srcs = ["testing/counter_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "cv_mu_timeout_stress_cpp_test",
    size = "small",
    srcs = ["testing/cv_mu_timeout_stress_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "cv_cpp_test",
    size = "small",
    srcs = ["testing/cv_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "cv_wait_example_cpp_test",
    size = "small",
    srcs = ["testing/cv_wait_example_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "dll_cpp_test",
    size = "small",
    srcs = ["testing/dll_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "mu_starvation_cpp_test",
    size = "small",
    srcs = ["testing/mu_starvation_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "mu_cpp_test",
    size = "small",
    srcs = ["testing/mu_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "mu_wait_example_cpp_test",
    size = "small",
    srcs = ["testing/mu_wait_example_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "mu_wait_cpp_test",
    size = "small",
    srcs = ["testing/mu_wait_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "note_cpp_test",
    size = "small",
    srcs = ["testing/note_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "once_cpp_test",
    size = "small",
    srcs = ["testing/once_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "pingpong_cpp_test",
    size = "small",
    srcs = ["testing/pingpong_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)

cc_test(
    name = "wait_cpp_test",
    size = "small",
    srcs = ["testing/wait_test.c"],
    copts = NSYNC_OPTS_CPP,
    linkopts = NSYNC_LINK_OPTS_CPP,
    deps = [
        ":nsync_cpp",
        ":nsync_test_lib_cpp",
    ],
)
