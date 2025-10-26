/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#include "fastcond.h"

#include <assert.h>
#include <errno.h>

/* Windows needs time.h for struct timespec (C11) */
#ifdef FASTCOND_USE_WINDOWS
#include <time.h>
#endif

/* Include sched.h for sched_yield() on POSIX platforms */
#ifndef FASTCOND_USE_WINDOWS
#include <sched.h>
#endif

#ifdef FASTCOND_TEST_INSTRUMENTATION
/*
 * Test instrumentation callback
 * This global callback is invoked by fastcond functions when
 * FASTCOND_TEST_INSTRUMENTATION is defined, allowing tests to verify
 * that patched code actually calls fastcond implementations.
 */
static fastcond_test_callback_t _test_callback = NULL;

FASTCOND_API(void)
fastcond_set_test_callback(fastcond_test_callback_t callback)
{
    _test_callback = callback;
}

FASTCOND_API(fastcond_test_callback_t)
fastcond_get_test_callback(void)
{
    return _test_callback;
}

/* Helper macro to invoke test callback if registered */
#define TEST_CALLBACK(func_name)                                                                   \
    do {                                                                                           \
        if (_test_callback)                                                                        \
            _test_callback(func_name);                                                             \
    } while (0)
#else
/* No-op when instrumentation disabled */
#define TEST_CALLBACK(func_name) ((void) 0)
#endif /* FASTCOND_TEST_INSTRUMENTATION */

/* Platform-specific semaphore wrapper macros for cleaner code */
#ifdef FASTCOND_USE_WINDOWS
/* Windows Semaphore Objects
 * CreateSemaphore creates a semaphore with initial count 0, max count LONG_MAX
 * WaitForSingleObject waits for semaphore to be signaled (count > 0)
 * ReleaseSemaphore increments semaphore count, waking waiting threads
 */
#define SEM_INIT(sem) (((sem) = CreateSemaphoreW(NULL, 0, LONG_MAX, NULL)) ? 0 : ENOMEM)
#define SEM_DESTROY(sem) (CloseHandle(sem) ? 0 : EINVAL)
#define SEM_WAIT(sem) (WaitForSingleObject((sem), INFINITE) == WAIT_OBJECT_0 ? 0 : EINVAL)
#define SEM_TIMEDWAIT(sem, abstime) _sem_timedwait_windows((sem), (abstime))
#define SEM_POST(sem) (ReleaseSemaphore((sem), 1, NULL) ? 0 : EINVAL)

/* Helper function to convert absolute timespec to relative timeout and wait */
static int _sem_timedwait_windows(HANDLE sem, const struct timespec *abstime)
{
    DWORD timeout_ms;

    if (!abstime) {
        /* NULL means infinite wait */
        return (WaitForSingleObject(sem, INFINITE) == WAIT_OBJECT_0) ? 0 : EINVAL;
    }

    /* Convert absolute timespec to relative timeout in milliseconds */
    struct timespec now;
    timespec_get(&now, TIME_UTC); /* C11 standard, widely supported */

    long long ns_diff =
        (abstime->tv_sec - now.tv_sec) * 1000000000LL + (abstime->tv_nsec - now.tv_nsec);

    if (ns_diff <= 0) {
        /* Already timed out */
        timeout_ms = 0;
    } else {
        /* Convert nanoseconds to milliseconds, rounding up */
        timeout_ms = (DWORD) ((ns_diff + 999999) / 1000000);
    }

    DWORD result = WaitForSingleObject(sem, timeout_ms);
    if (result == WAIT_OBJECT_0) {
        return 0; /* Success */
    } else if (result == WAIT_TIMEOUT) {
        return ETIMEDOUT;
    } else {
        return EINVAL; /* Error */
    }
}

#elif defined(FASTCOND_USE_GCD)
/* GCD semaphores for macOS
 * dispatch_semaphore_wait returns 0 on success (semaphore decremented)
 * dispatch_semaphore_wait returns non-zero on timeout
 * dispatch_semaphore_signal returns non-zero if thread was woken, always succeeds
 */
#define SEM_INIT(sem) ((sem) = dispatch_semaphore_create(0), (sem) ? 0 : ENOMEM)
#define SEM_DESTROY(sem) (dispatch_release(sem), 0)
#define SEM_WAIT(sem) (dispatch_semaphore_wait((sem), DISPATCH_TIME_FOREVER))
#define SEM_TIMEDWAIT(sem, abstime) _sem_timedwait_gcd((sem), (abstime))
#define SEM_POST(sem) (dispatch_semaphore_signal(sem), 0)

/* Helper function to convert absolute timespec to dispatch_time_t and wait */
static int _sem_timedwait_gcd(dispatch_semaphore_t sem, const struct timespec *abstime)
{
    dispatch_time_t timeout;

    if (!abstime) {
        /* NULL means infinite wait */
        return dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    }

    /* Convert absolute timespec to dispatch_time_t */
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    long long ns_diff =
        (abstime->tv_sec - now.tv_sec) * 1000000000LL + (abstime->tv_nsec - now.tv_nsec);

    if (ns_diff <= 0) {
        /* Already timed out */
        timeout = DISPATCH_TIME_NOW;
    } else {
        timeout = dispatch_time(DISPATCH_TIME_NOW, ns_diff);
    }

    long result = dispatch_semaphore_wait(sem, timeout);
    /* dispatch_semaphore_wait returns 0 on success, non-zero on timeout */
    return result ? ETIMEDOUT : 0;
}
#else
/* POSIX semaphores for Linux and other Unix systems */
#define SEM_INIT(sem) (sem_init(&(sem), 0, 0) ? errno : 0)
#define SEM_DESTROY(sem) (sem_destroy(&(sem)) ? errno : 0)
#define SEM_WAIT(sem) (sem_wait(&(sem)) ? errno : 0)
#define SEM_TIMEDWAIT(sem, abstime)                                                                \
    ((abstime) ? (sem_timedwait(&(sem), (abstime)) ? errno : 0) : SEM_WAIT(sem))
#define SEM_POST(sem) (sem_post(&(sem)) ? errno : 0)
#endif

/* Platform-specific thread yield */
#ifdef FASTCOND_USE_WINDOWS
#define YIELD() Sleep(0) /* Windows: Sleep(0) yields to other ready threads */
#else
#define YIELD() sched_yield() /* POSIX: sched_yield() */
#endif

/* Optional: disable yield in strong condition variable spurious wakeup path
 * Set FASTCOND_NO_YIELD=1 to test performance without scheduler yield.
 * The yield is used when n_wakeup > 0 to give already-signaled threads
 * a better chance to wake up before this thread re-acquires the mutex.
 * Removing it may slightly improve latency but could theoretically increase
 * wakeup unfairness under heavy contention.
 */
#ifndef FASTCOND_NO_YIELD
#define MAYBE_YIELD() YIELD()
#else
#define MAYBE_YIELD() ((void) 0) /* No-op */
#endif

/*  fastcond_cond_t implementation - Unified strong condition variable

    Historical Note:
    Earlier versions of this library provided two variants:
    - fastcond_wcond_t: "weak" semantics (could wake signalling thread, violating POSIX)
    - fastcond_cond_t: "strong" semantics (guarantees only waiting threads wake)

    Performance benchmarking showed the strong variant is actually FASTER than weak
    despite additional bookkeeping, while also providing correct POSIX semantics.
    Therefore, the API has been unified - fastcond_wcond_t is now an alias for
    fastcond_cond_t, and both provide strong semantics.

    Implementation Strategy:
    This implementation uses a layered approach for conceptual clarity:
    1. Static inline helpers (_weak_*) provide the basic semaphore primitive layer
    2. Public fastcond_cond_* functions add strong semantics via bookkeeping
    3. Backward-compatible fastcond_wcond_* aliases call the strong implementation

    The weak primitive layer corresponds to the simple emulation in Birrell's paper
    (listing 2: http://birrell.org/andrew/papers/ImplementingCVs.pdf).
    It uses a semaphore + waiting counter but allows wakeup stealing.

    The strong layer adds n_wakeup tracking to prevent newly-arriving threads from
    stealing wakeups intended for already-waiting threads, guaranteeing POSIX semantics
    at the cost of occasional spurious wakeups.

    Background:
    The semaphore-based emulation comes from work on Windows CriticalSection objects.
    Semaphores (available Windows XP+, POSIX) provide counted state unlike auto-reset
    events, preventing lost-wakeup race conditions when multiple threads are interrupted.

    Note: Unlike pthread_cond_*, these wait functions are NOT cancellation points.
    Emulating that would require pthread_testcancel() and EINTR handling.

    Spurious Wakeups:
    Condition variables do not guarantee the predicate is true on wakeup - threads may
    wake due to signals, internal bookkeeping, etc. Callers must always retest their
    predicate in a loop. This freedom simplifies both implementation and usage.
*/

/* Static inline helpers implementing the basic weak primitive layer.
 * These provide simple semaphore-based signaling with waiting counter,
 * but allow wakeup stealing (signalling thread may wake itself).
 * Used internally by the strong implementation below.
 */

static inline int _weak_init(fastcond_cond_t *cond)
{
    cond->w_waiting = 0;
    return SEM_INIT(cond->sem);
}

static inline int _weak_fini(fastcond_cond_t *cond)
{
    return SEM_DESTROY(cond->sem);
}

static inline int _weak_timedwait(fastcond_cond_t *cond, native_mutex_t *restrict mutex,
                                  const struct timespec *restrict abstime)
{
    int err1, err2;
    cond->w_waiting++;
    err1 = NATIVE_MUTEX_UNLOCK(mutex);
    if (err1)
        return err1;

    err1 = SEM_TIMEDWAIT(cond->sem, abstime);
    err2 = NATIVE_MUTEX_LOCK(mutex);

    if (err1)
        /* wakeup did not adjust counter, must do it ourselves */
        --cond->w_waiting;

    if (err1 == EINTR)
        err1 = 0; /* signals, etc, cause spurious wakeup */

    if (err2)
        return err2;
    return err1;
}

static inline int _weak_signal(fastcond_cond_t *cond)
{
    if (cond->w_waiting > 0) {
        int err = SEM_POST(cond->sem);
        if (err)
            return err;
        cond->w_waiting--;
    }
    return 0;
}

static inline int _weak_broadcast(fastcond_cond_t *cond)
{
    while (cond->w_waiting > 0) {
        int err = SEM_POST(cond->sem);
        if (err)
            return err;
        cond->w_waiting--;
    }
    return 0;
}

/* Strong condition variable implementation using weak primitive helpers.
 * Adds n_wakeup bookkeeping to ensure only already-waiting threads receive wakeups.
 *
 * Key mechanism: If n_wakeup > 0 (pending wakeups), new waiters immediately return
 * with spurious wakeup instead of stealing the semaphore from waiting threads.
 * This preserves POSIX semantics: signal/broadcast only wakes existing waiters.
 *
 * Invariant maintained: n_wakeup <= waiting (cannot have more pending wakeups than waiters)
 */

FASTCOND_API(int)
fastcond_cond_init(fastcond_cond_t *restrict cond, const void *restrict attr)
{
    TEST_CALLBACK("fastcond_cond_init");
    (void) attr; /* Unused - condattr not supported */
    cond->n_waiting = 0;
    cond->n_wakeup = 0;
    return _weak_init(cond);
}

FASTCOND_API(int)
fastcond_cond_fini(fastcond_cond_t *cond)
{
    return _weak_fini(cond);
}

FASTCOND_API(int)
fastcond_cond_wait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex)
{
    TEST_CALLBACK("fastcond_cond_wait");
    return fastcond_cond_timedwait(cond, mutex, 0);
}

FASTCOND_API(int)
fastcond_cond_timedwait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex,
                        const struct timespec *restrict abstime)
{
    int err;
    assert(cond->n_wakeup <= cond->n_waiting);

    if (cond->n_wakeup) {
        /* Pending wakeups exist for threads already waiting.
         * Cannot enter wait state - would steal wakeup from them, violating
         * strong semantics (only already-waiting threads should wake).
         * Instead, perform spurious wakeup (allowed by CV protocol) while
         * yielding lock to let signalled threads complete their wakeup.
         */
        err = NATIVE_MUTEX_UNLOCK(mutex);
        if (err)
            return err;
        MAYBE_YIELD();
        return NATIVE_MUTEX_LOCK(mutex);
    }

    /* No pending wakeups - safe to wait using weak primitive.
     * Track at strong layer (n_waiting) separately from weak layer (waiting).
     */
    cond->n_waiting++;
    err = _weak_timedwait(cond, mutex, abstime);
    cond->n_waiting--;

    /* If we were woken by signal/broadcast, consume the pending wakeup marker */
    if (cond->n_wakeup > 0)
        cond->n_wakeup--;

    return err;
}

static int _fastcond_cond_signal_n(fastcond_cond_t *cond, int n)
{
    int err = 0;
    int unwoken = cond->n_waiting - cond->n_wakeup; /* threads waiting without pending wakeup */

    if (unwoken > 0) {
        int to_wake;

        /* Determine how many threads to wake:
         * n == 1: wake one thread (signal)
         * n < 0: wake all (broadcast)
         * n > 1: wake min(n, unwoken) threads
         */
        if (n == 1 || unwoken == 1) {
            err = _weak_signal(cond);
            to_wake = 1;
        } else if (n > 0 && n < unwoken) {
            int i;
            for (i = 0; err == 0 && i < n; i++)
                err = _weak_signal(cond);
            to_wake = (err == 0) ? n : i;
        } else {
            err = _weak_broadcast(cond);
            to_wake = unwoken;
        }

        /* Track pending wakeups to prevent wakeup stealing by new waiters */
        if (err == 0)
            cond->n_wakeup += to_wake;
    }
    return err;
}

FASTCOND_API(int)
fastcond_cond_signal(fastcond_cond_t *cond)
{
    TEST_CALLBACK("fastcond_cond_signal");
    return _fastcond_cond_signal_n(cond, 1);
}

FASTCOND_API(int)
fastcond_cond_broadcast(fastcond_cond_t *cond)
{
    return _fastcond_cond_signal_n(cond, -1);
}

/* Backward-compatible weak condition variable API.
 * These are now simple aliases to the strong implementation.
 * All fastcond_wcond_* functions now provide strong POSIX semantics.
 */

FASTCOND_API(int)
fastcond_wcond_init(fastcond_wcond_t *restrict cond, const void *restrict attr)
{
    TEST_CALLBACK("fastcond_wcond_init");
    return fastcond_cond_init(cond, attr);
}

FASTCOND_API(int)
fastcond_wcond_fini(fastcond_wcond_t *cond)
{
    return fastcond_cond_fini(cond);
}

FASTCOND_API(int)
fastcond_wcond_wait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex)
{
    TEST_CALLBACK("fastcond_wcond_wait");
    return fastcond_cond_wait(cond, mutex);
}

FASTCOND_API(int)
fastcond_wcond_timedwait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex,
                         const struct timespec *restrict abstime)
{
    return fastcond_cond_timedwait(cond, mutex, abstime);
}

FASTCOND_API(int)
fastcond_wcond_signal(fastcond_wcond_t *cond)
{
    TEST_CALLBACK("fastcond_wcond_signal");
    return fastcond_cond_signal(cond);
}

FASTCOND_API(int)
fastcond_wcond_broadcast(fastcond_wcond_t *cond)
{
    return fastcond_cond_broadcast(cond);
}

#ifdef FASTCOND_USE_WINDOWS
/*
 * Windows-specific millisecond-based wait functions.
 * These bypass the timespec conversion and directly use WaitForSingleObject
 * with the millisecond timeout, which is more efficient for Windows code.
 */

/* Helper: wait on semaphore with millisecond timeout */
static int _sem_wait_ms(HANDLE sem, DWORD timeout_ms)
{
    DWORD result = WaitForSingleObject(sem, timeout_ms);
    if (result == WAIT_OBJECT_0) {
        return 0; /* Success */
    } else if (result == WAIT_TIMEOUT) {
        return ETIMEDOUT;
    } else {
        return EINVAL; /* Error */
    }
}

FASTCOND_API(int)
fastcond_wcond_wait_ms(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex,
                       DWORD timeout_ms)
{
    TEST_CALLBACK("fastcond_wcond_wait_ms");
    /* wcond is now an alias for cond - just call the strong implementation */
    return fastcond_cond_wait_ms(cond, mutex, timeout_ms);
}

FASTCOND_API(int)
fastcond_cond_wait_ms(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex,
                      DWORD timeout_ms)
{
    TEST_CALLBACK("fastcond_cond_wait_ms");
    int err;

    assert(cond->n_wakeup <= cond->n_waiting);

    if (cond->n_wakeup) {
        /* Pending wakeups - perform spurious wakeup instead of stealing */
        err = NATIVE_MUTEX_UNLOCK(mutex);
        if (err)
            return err;
#ifdef _MSC_VER
        Sleep(0); /* Yield on Windows */
#else
        sched_yield();
#endif
        return NATIVE_MUTEX_LOCK(mutex);
    }

    /* No pending wakeups - use weak primitive with millisecond timeout */
    int err1, err2;
    cond->n_waiting++;
    cond->w_waiting++;
    err1 = NATIVE_MUTEX_UNLOCK(mutex);
    if (err1)
        return err1;

    err1 = _sem_wait_ms(cond->sem, timeout_ms);
    err2 = NATIVE_MUTEX_LOCK(mutex);

    if (err1)
        /* wakeup did not adjust counter, must do it ourselves */
        --cond->w_waiting;

    if (err1 == EINTR)
        err1 = 0; /* signals, etc, cause spurious wakeup */

    cond->n_waiting--;
    if (cond->n_wakeup > 0)
        cond->n_wakeup--;

    if (err2)
        return err2;
    return err1;
}
#endif /* FASTCOND_USE_WINDOWS */
