# -*- mode: python; -*-

package(
    default_visibility = ["//visibility:public"],
)

# -----------
# Parameters to the compilation:  compiler (e.g., for atomics), architecture
# (e.g., for load and store barrier behaviour), and OS.
# blaze/bazel merge these into one, somewhat slippery, "cpu" string.

config_setting(
    name = "gcc_linux_x86_64_1",
    values = {"cpu": "k8"},
)

config_setting(
    name = "gcc_linux_x86_64_2",
    values = {"cpu": "haswell"},
)

config_setting(
    name = "gcc_linux_x86_32",
    values = {"cpu": "piii"},
)

config_setting(
    name = "gcc_linux_aarch64",
    values = {"cpu": "arm"},
)

config_setting(
    name = "gcc_linux_ppc64",
    values = {"cpu": "ppc"},
)

# cpu "x64_windows_msvc" is Windows and x86_64, and msvc.
# cpu "x64_windows" is Windows and x86_64, with an unknown compiler.
# cpu "darwin" is OS X and x86_64, probably xcode compiler.
# cpu "freebsd" is freebsd, probably gcc, but the CPU is unclear.
# cpu "armeabi-v7a" is arm32, probably gcc, probably Android.

# -----------
# Compilation options.

NSYNC_OPTS = select({
    # Select the OS include directory.
    ":gcc_linux_x86_64_1": ["-I" + PACKAGE_NAME + "/platform/linux"],
    ":gcc_linux_x86_64_2": ["-I" + PACKAGE_NAME + "/platform/linux"],
    ":gcc_linux_x86_32": ["-I" + PACKAGE_NAME + "/platform/linux"],
    ":gcc_linux_aarch64": ["-I" + PACKAGE_NAME + "/platform/linux"],
    ":gcc_linux_ppc64": ["-I" + PACKAGE_NAME + "/platform/linux"],
}) + select({
    # Select the compiler include directory.
    ":gcc_linux_x86_64_1": ["-I" + PACKAGE_NAME + "/platform/gcc"],
    ":gcc_linux_x86_64_2": ["-I" + PACKAGE_NAME + "/platform/gcc"],
    ":gcc_linux_x86_32": ["-I" + PACKAGE_NAME + "/platform/gcc"],
    ":gcc_linux_aarch64": ["-I" + PACKAGE_NAME + "/platform/gcc"],
    ":gcc_linux_ppc64": ["-I" + PACKAGE_NAME + "/platform/gcc"],
}) + select({
    # Select the CPU architecture include directory.
    ":gcc_linux_x86_64_1": ["-I" + PACKAGE_NAME + "/platform/x86_64"],
    ":gcc_linux_x86_64_2": ["-I" + PACKAGE_NAME + "/platform/x86_64"],
    ":gcc_linux_x86_32": ["-I" + PACKAGE_NAME + "/platform/x86_32"],
    ":gcc_linux_aarch64": ["-I" + PACKAGE_NAME + "/platform/aarch64"],
    ":gcc_linux_ppc64": ["-I" + PACKAGE_NAME + "/platform/ppc64"],
}) + [
    "-I" + PACKAGE_NAME + "/public",
    "-I" + PACKAGE_NAME + "/internal",
    "-I" + PACKAGE_NAME + "/platform/posix",
    "-D_POSIX_C_SOURCE=200809L",
]

# -----------
# Header files the source may include.

NSYNC_INTERNAL_HEADERS = [
    "internal/common.h",
    "internal/dll.h",
    "internal/headers.h",
    "internal/sem.h",
    "internal/wait_internal.h",
]

NSYNC_TEST_HEADERS = NSYNC_INTERNAL_HEADERS + [
    "testing/array.h",
    "testing/atm_log.h",
    "testing/closure.h",
    "testing/heap.h",
    "testing/smprintf.h",
    "testing/testing.h",
    "testing/time_extra.h",
]

# This declares headers for all platforms, not just the one
# we're building for, to avoid a more complex build file.
NSYNC_INTERNAL_HEADERS_PLATFORM = [
    "platform/aarch64/cputype.h",
    "platform/alpha/cputype.h",
    "platform/arm/cputype.h",
    "platform/atomic_ind/atomic.h",
    "platform/c++11/atomic.h",
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
    "platform/msvc/compiler.h",
    "platform/netbsd/atomic.h",
    "platform/netbsd/platform.h",
    "platform/openbsd/platform.h",
    "platform/osf1/platform.h",
    "platform/macos/atomic.h",
    "platform/macos/platform.h",
    "platform/pmax/cputype.h",
    "platform/posix/nsync_time_init.h",
    "platform/ppc32/cputype.h",
    "platform/ppc64/cputype.h",
    "platform/shark/cputype.h",
    "platform/tcc/compiler.h",
    "platform/win32/platform.h",
    "platform/x86_32/cputype.h",
    "platform/x86_64/cputype.h",
]

# -----------
# The nsync library.

NSYNC_SRC_LINUX = [
    "platform/linux/src/nsync_semaphore_futex.c",
    "platform/posix/src/per_thread_waiter.c",
    "platform/posix/src/yield.c",
    "platform/posix/src/time_rep.c",
    "platform/posix/src/nsync_panic.c",
]

NSYNC_SRC_PLATFORM = select({
    ":gcc_linux_x86_64_1": NSYNC_SRC_LINUX,
    ":gcc_linux_x86_64_2": NSYNC_SRC_LINUX,
    ":gcc_linux_x86_32": NSYNC_SRC_LINUX,
    ":gcc_linux_aarch64": NSYNC_SRC_LINUX,
    ":gcc_linux_ppc64": NSYNC_SRC_LINUX,
})

cc_library(
    name = "nsync",
    srcs = [
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
    ] + NSYNC_SRC_PLATFORM,
    hdrs = [
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
    ],
    copts = NSYNC_OPTS,
    linkopts = ["-pthread"],
    textual_hdrs = NSYNC_INTERNAL_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
)

# -----------
# Tests.

NSYNC_TEST_SRC_LINUX = [
    "platform/posix/src/start_thread.c",
]

NSYNC_TEST_SRC_PLATFORM = select({
    ":gcc_linux_x86_64_1": NSYNC_TEST_SRC_LINUX,
    ":gcc_linux_x86_64_2": NSYNC_TEST_SRC_LINUX,
    ":gcc_linux_x86_32": NSYNC_TEST_SRC_LINUX,
    ":gcc_linux_aarch64": NSYNC_TEST_SRC_LINUX,
    ":gcc_linux_ppc64": NSYNC_TEST_SRC_LINUX,
})

cc_library(
    name = "nsync_test_lib",
    srcs = [
        "testing/array.c",
        "testing/atm_log.c",
        "testing/closure.c",
        "testing/smprintf.c",
        "testing/testing.c",
        "testing/time_extra.c",
    ] + NSYNC_TEST_SRC_PLATFORM,
    hdrs = ["testing/testing.h"],
    copts = NSYNC_OPTS,
    textual_hdrs = NSYNC_TEST_HEADERS + NSYNC_INTERNAL_HEADERS_PLATFORM,
    deps = [":nsync"],
)

cc_test(
    name = "counter_test",
    size = "small",
    srcs = ["testing/counter_test.c"],
    copts = NSYNC_OPTS,
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
    deps = [
        ":nsync",
        ":nsync_test_lib",
    ],
)
