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

/**
 * CRITICAL USAGE REQUIREMENT: Mutex Must Be Held
 * ==============================================
 *
 * Unlike some condition variable implementations, fastcond requires that the
 * associated mutex be held when calling signal or broadcast. Calling these
 * functions without holding the mutex produces UNDEFINED BEHAVIOR.
 *
 * This matches the original intent in Birrell's seminal paper "Implementing
 * Condition Variables with Semaphores" (DEC SRC-RR-1995-004) and POSIX's
 * historical design assumptions, though POSIX.1-2008 later clarified that
 * signal/broadcast without the lock is technically permitted (albeit with
 * reduced guarantees).
 *
 * Required pattern:
 *     native_mutex_lock(&mutex);
 *     // ... modify shared state ...
 *     fastcond_cond_signal(&cond);   // Mutex MUST be held
 *     native_mutex_unlock(&mutex);
 *
 * The rationale: fastcond's internal bookkeeping (n_waiting, n_wakeup) is
 * NOT protected by atomic operations. The algorithm assumes the calling thread
 * holds the associated mutex, which provides the necessary memory ordering and
 * mutual exclusion for correct operation. Violating this assumption creates
 * race conditions in the counter updates.
 *
 * This differs from implementations like Linux's futex-based pthread_cond_t,
 * which use atomic operations for internal state and can safely handle
 * signal/broadcast without the lock held (though POSIX still recommends
 * holding it for predictable scheduling behavior).
 */

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

/* Signal one waiting thread. CRITICAL: The associated mutex MUST be held.
 * Calling without the mutex produces undefined behavior. */
FASTCOND_API(int)
fastcond_cond_signal(fastcond_cond_t *cond);

/* Wake all waiting threads. CRITICAL: The associated mutex MUST be held.
 * Calling without the mutex produces undefined behavior. */
FASTCOND_API(int)
fastcond_cond_broadcast(fastcond_cond_t *cond);

/* The weak condition variable API is now an alias for strong
 *
 * Historical note: The original fastcond implementation (2017) introduced both
 * "weak" and "strong" semantic variants. The weak variant relaxed POSIX semantics,
 * allowing signal/broadcast to wake ANY thread (including newly-arriving ones that
 * haven't started waiting yet). The rationale was that simpler bookkeeping might
 * offer better performance for use cases where all waiting threads are equivalent.
 *
 * However, rigorous cross-platform performance testing (2025) disproved this theory.
 * The strong variant consistently outperforms weak across all benchmarks, despite
 * maintaining additional counters (n_waiting, n_wakeup) to prevent wakeup stealing.
 * The bookkeeping overhead is more than offset by reduced contention and better
 * cache behavior.
 *
 * Since strong is both faster AND semantically correct (proper POSIX behavior),
 * maintaining weak as a separate implementation serves no purpose. As of v0.3.0,
 * fastcond_wcond_t is a typedef alias of fastcond_cond_t.
 *
 * The wcond API is kept for backwards compatibility - existing code continues to
 * work unchanged and automatically receives the performance and correctness benefits
 * of strong semantics.
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

/* Signal one waiting thread. CRITICAL: The associated mutex MUST be held.
 * Calling without the mutex produces undefined behavior. */
FASTCOND_API(int)
fastcond_wcond_signal(fastcond_wcond_t *cond);

/* Wake all waiting threads. CRITICAL: The associated mutex MUST be held.
 * Calling without the mutex produces undefined behavior. */
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
