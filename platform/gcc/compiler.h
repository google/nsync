#ifndef NSYNC_PLATFORM_GCC_COMPILER_H_
#define NSYNC_PLATFORM_GCC_COMPILER_H_

#define INLINE __inline
#define UNUSED __attribute__((unused))
#define THREAD_LOCAL __thread
#define HAVE_THREAD_LOCAL 1

#endif /*NSYNC_PLATFORM_GCC_COMPILER_H_*/
