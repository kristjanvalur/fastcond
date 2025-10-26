/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#ifndef _FASTCOND_H_
#define _FASTCOND_H_

/* Version information */
#define FASTCOND_VERSION_MAJOR 0
#define FASTCOND_VERSION_MINOR 3
#define FASTCOND_VERSION_PATCH 0

/* Semantic version string */
#define FASTCOND_VERSION "0.3.0"

/* Numeric version for compile-time comparisons: MAJOR*10000 + MINOR*100 + PATCH */
#define FASTCOND_VERSION_NUM                                                                       \
    (FASTCOND_VERSION_MAJOR * 10000 + FASTCOND_VERSION_MINOR * 100 + FASTCOND_VERSION_PATCH)

/* Use native primitives abstraction for platform-specific mutex types */
#include "native_primitives.h"

/* Platform detection and synchronization primitive selection */
#if defined(_WIN32) || defined(_WIN64)
/* Windows platform */
#define FASTCOND_USE_WINDOWS 1
#include <windows.h>
#elif defined(__APPLE__) || defined(__MACH__)
/* macOS platform - use GCD dispatch semaphores */
#include <dispatch/dispatch.h>
#define FASTCOND_USE_GCD 1
#else
/* POSIX platforms (Linux, BSD, etc.) */
#include <semaphore.h>
#endif

#define FASTCOND_API(v) v

/* The strong condition variable - primary implementation with full POSIX semantics
 * This is the main condition variable type with correct wakeup guarantees.
 */

typedef struct _fastcond_cond_t {
#ifdef FASTCOND_USE_WINDOWS
    HANDLE sem; /* Windows semaphore handle */
#elif defined(FASTCOND_USE_GCD)
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
    volatile int w_waiting; /* weak layer: threads blocked on semaphore */
    volatile int n_waiting; /* strong layer: threads in wait (including spurious wakeups) */
    volatile int n_wakeup;  /* strong layer: awoken threads that haven't exited yet */
} fastcond_cond_t;

FASTCOND_API(int)
fastcond_cond_init(fastcond_cond_t *restrict cond, const void *restrict attr);

FASTCOND_API(int)
fastcond_cond_fini(fastcond_cond_t *cond);

FASTCOND_API(int)
fastcond_cond_wait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex);

FASTCOND_API(int)
fastcond_cond_timedwait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex,
                        const struct timespec *restrict abstime);

FASTCOND_API(int)
fastcond_cond_signal(fastcond_cond_t *cond);

FASTCOND_API(int)
fastcond_cond_broadcast(fastcond_cond_t *cond);

/* The weak condition variable API is now an alias for strong
 *
 * Historical note: The original fastcond implementation (2017) used "weak"
 * semantics that could wake threads not yet waiting (wakeup stealing).
 * The strong variant was developed to fix this with spurious wakeup prevention.
 * Performance testing showed strong is actually FASTER than weak, so we've
 * unified on the strong implementation.
 *
 * The wcond API is kept for backwards compatibility but now provides strong
 * semantics - which is strictly better! Code using wcond will continue to work
 * and will automatically get correct POSIX semantics.
 */
typedef fastcond_cond_t fastcond_wcond_t;

FASTCOND_API(int)
fastcond_wcond_init(fastcond_wcond_t *restrict cond, const void *restrict attr);

FASTCOND_API(int)
fastcond_wcond_fini(fastcond_wcond_t *cond);

FASTCOND_API(int)
fastcond_wcond_wait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex);

FASTCOND_API(int)
fastcond_wcond_timedwait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex,
                         const struct timespec *restrict abstime);

FASTCOND_API(int)
fastcond_wcond_signal(fastcond_wcond_t *cond);

FASTCOND_API(int)
fastcond_wcond_broadcast(fastcond_wcond_t *cond);

#ifdef FASTCOND_TEST_INSTRUMENTATION
/*
 * Test instrumentation for validating fastcond_patch.h
 *
 * When FASTCOND_TEST_INSTRUMENTATION is defined, fastcond functions will
 * call a registered callback on entry, allowing tests to verify that
 * patched code actually invokes fastcond implementations rather than
 * native condition variable functions.
 *
 * This is only for testing - never define in production code.
 */
typedef void (*fastcond_test_callback_t)(const char *function_name);

FASTCOND_API(void)
fastcond_set_test_callback(fastcond_test_callback_t callback);

FASTCOND_API(fastcond_test_callback_t)
fastcond_get_test_callback(void);
#endif /* FASTCOND_TEST_INSTRUMENTATION */

#ifdef FASTCOND_USE_WINDOWS
/*
 * Windows-specific variants that take timeout in milliseconds directly,
 * matching the Windows CONDITION_VARIABLE API signature.
 * These are more efficient than the timespec variants on Windows as they
 * avoid the absolute-to-relative time conversion overhead.
 *
 * timeout_ms: Timeout in milliseconds, or INFINITE for no timeout
 * Returns: 0 on success, ETIMEDOUT on timeout, or other errno value on error
 */
FASTCOND_API(int)
fastcond_wcond_wait_ms(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex,
                       DWORD timeout_ms);

FASTCOND_API(int)
fastcond_cond_wait_ms(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex,
                      DWORD timeout_ms);
#endif /* FASTCOND_USE_WINDOWS */

#endif /* ! defined _FASTCOND_H_ */
