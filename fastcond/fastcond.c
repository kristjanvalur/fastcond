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

/*  The fastcond_wcond_t code - weak condition variable (see below for `weak`)

    The following comes out of work to create a working CriticalSection
    object on earlier versions of Windows.

    Windows:
    The mutex is a CriticalSection.
    The condition variable is emulated with the help of a semaphore.
    Semaphores are available on Windows XP (2003 server) and later.
    We use a Semaphore rather than an auto-reset event, because although
    an auto-resent event might appear to solve the lost-wakeup bug (race
    condition between releasing the outer lock and waiting) because it
    maintains state even though a wait hasn't happened, there is still
    a lost wakeup problem if more than one thread are interrupted in the
    critical place.  A semaphore solves that, because its state is counted,
    not Boolean.
    Because it is ok to signal a condition variable with no one
    waiting, we need to keep track of the number of
    waiting threads.  Otherwise, the semaphore's state could rise
    without bound.  This also helps reduce the number of `spurious wakeups`
    that would otherwise happen (see below).

    Posix:
    Here we stick to pthread_mutex_t as the mutex, but for fun and profit,
    decide to emulate a condition variable in the same manner using the
    posix semaphore.
    Notice however that unlike `real` pthread semaphores, the wait
    functions here are _not_ pthread cancellation points.  Perhaps it would
    be possible to emulate that using pthread_checkcancel, and watching EINTR
    result values.  But we won't go there.

    `Weak` condition
    This implementation still has the problem that the threads woken
    with a "signal" aren't necessarily those that are already
    waiting.  It corresponds to listing 2 in:
    http://birrell.org/andrew/papers/ImplementingCVs.pdf
    In particular, condition variables promise that a "signal" wakes
    _at least_ one of the already waiting threads, and that a broadcast
    wakes _all_ of the waiting threads.  This simple emulation cannot guarantee
    that because a newly arriving thread may "steal" the semaphore from
    the waiting threads.  In particular, the thread that makes the 'signal'
    may subsequently 'wait', but wake itself up, rather than one of the
    other threads.

    For many cases, this is fine.  Whenever it is sufficient that _a_ thread is
    indeed awoken, a weak condition variable works well.  This is the case if
    all the threads that can wait are equivalent.
    But for applications requiring the strict semantics of condition variables
    we need to add additional features to guarantee that behaviour.
    See PySCOND below for details.

    `spurious wakeups`
    The Condition Variable protocol sensible does not guarantee that when
    a thread wakes up, Its predicate is true.  In other words, it may wake up
    due to some unspecified reasons (such as signals) and the caller of wait functions
    must always test that a predicate is true (and it can stop waiting) when returning
    from wait functions.  This freedom makes implementation of condition variables
    _much_ simpler and also the logic concerning their use.  Caller need not
    reason about why he woke up, he simply tests the predicates.  Condition variables
    provide a guarantee to wake up, but not a guarantee to sleep.

    Generic emulations of the pthread_cond_* API using
    earlier Win32 functions can be found on the Web.
    The following read can be give background information to these issues,
    but the implementations are all broken in some way.
    http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
*/

FASTCOND_API(int)
fastcond_wcond_init(fastcond_wcond_t *restrict cond, const void *restrict attr)
{
    (void) attr; /* Unused - condattr not supported */
    cond->waiting = 0;
    return SEM_INIT(cond->sem);
}

FASTCOND_API(int)
fastcond_wcond_fini(fastcond_wcond_t *cond)
{
    return SEM_DESTROY(cond->sem);
}

FASTCOND_API(int)
fastcond_wcond_wait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex)
{
    return fastcond_wcond_timedwait(cond, mutex, 0);
}

FASTCOND_API(int)
fastcond_wcond_timedwait(fastcond_wcond_t *restrict cond, native_mutex_t *restrict mutex,
                         const struct timespec *restrict abstime)
{
    int err1, err2;
    cond->waiting++;
    err1 = NATIVE_MUTEX_UNLOCK(mutex);
    if (err1)
        return err1;

    err1 = SEM_TIMEDWAIT(cond->sem, abstime);
    err2 = NATIVE_MUTEX_LOCK(mutex);

    if (err1)
        /* wakeup did not adjust this, must do it ourselves */
        --cond->waiting;

    if (err1 == EINTR)
        err1 = 0; /* signals, etc, cause spurious wakeup */

    if (err2)
        return err2;
    return err1;
}

FASTCOND_API(int)
fastcond_wcond_signal(fastcond_wcond_t *cond)
{
    if (cond->waiting > 0) {
        int err = SEM_POST(cond->sem);
        if (err)
            return err;
        cond->waiting--;
    }
    return 0;
}

FASTCOND_API(int)
fastcond_wcond_broadcast(fastcond_wcond_t *cond)
{
    while (cond->waiting > 0) {
        int err = SEM_POST(cond->sem);
        if (err)
            return err;
        cond->waiting--;
    }
    return 0;
}

/*  fastcond_cond_t code - the strong condition variable

    The above emulated condition variable, as explained, does make the full
    promise of the Condition Variable protocol, that a 'signal' wakes up at least
    one of the waiting threads.  It will wake up a thread, but it may be one that
    hasn't yet started waiting.
    This can cause problems if not all of the potentially waiting threads are
    equivalent, such as when servicing two ends of a pipe with a single condition
    variable.

    And so, we aim to provide a `strong` condition variable.
    The approach taken here is to provide additional bookkeeping outside
    keeping track of number of waiting threads
    inside it, and the number of wakeups that are `pending`.
    That is, threads that have been signalled to wake up but haven't yet exited.
    If a new thread attempts to wait while there are pending wakeups, it
    immediately returns without waiting, (but yields the cpu).  This prevents
    it from `stealing` the wakeup from one of the other threads.

    To the caller, this will look like a spurious wakeup, something that
    he must expect.  We can thus guarantee correctness, at the cost of
    some extra wakeups.
 */

FASTCOND_API(int)
fastcond_cond_init(fastcond_cond_t *restrict cond, const void *restrict attr)
{
    int err;
    cond->n_waiting = cond->n_wakeup = 0;
    err = fastcond_wcond_init(&cond->wait, attr);
    return err;
}

FASTCOND_API(int)
fastcond_cond_fini(fastcond_cond_t *cond)
{
    int err = fastcond_wcond_fini(&cond->wait);
    return err;
}

FASTCOND_API(int)
fastcond_cond_wait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex)
{
    return fastcond_cond_timedwait(cond, mutex, NULL);
}

FASTCOND_API(int)
fastcond_cond_timedwait(fastcond_cond_t *restrict cond, native_mutex_t *restrict mutex,
                        const struct timespec *restrict abstime)
{
    int err;
    assert(cond->n_wakeup <= cond->n_waiting);
    if (cond->n_wakeup) {
        /* there are signalled threads that haven't woken up.
         * I cannot take the chance of pre-empting them, which would violate
         * the strong condition that the threads already wating must be woken
         * up.  Therefore, we will perform a spurious wakeup (which is allowed)
         * while still allowing the waiting threads to wake up.
         * We must yield the lock to allow the other threads a chance to
         * wake up as well, throwing in a scheduler yield for good measure.
         */
        err = NATIVE_MUTEX_UNLOCK(mutex);
        if (err)
            return err;
        MAYBE_YIELD();
        return NATIVE_MUTEX_LOCK(mutex);
    }
    cond->n_waiting++;
    err = fastcond_wcond_timedwait(&cond->wait, mutex, abstime);
    cond->n_waiting--;
    if (cond->n_wakeup > 0)
        cond->n_wakeup--;
    return err;
}

static int _fastcond_cond_signal_n(fastcond_cond_t *cond, int n)
{
    int err = 0, unwoken = cond->n_waiting - cond->n_wakeup;
    if (unwoken > 0) {
        /* negative n means all, positive is the minimal amount to wake */
        if (n == 1 || unwoken == 1) {
            err = fastcond_wcond_signal(&cond->wait);
            n = 1;
        } else if (n > 0 && n < unwoken) {
            int i;
            for (i = 0; err == 0 && i < n; i++)
                err = fastcond_wcond_signal(&cond->wait);
        } else {
            err = fastcond_wcond_broadcast(&cond->wait);
            n = unwoken;
        }
        if (err == 0)
            cond->n_wakeup += n;
    }
    return err;
}

FASTCOND_API(int)
fastcond_cond_signal(fastcond_cond_t *cond)
{
    return _fastcond_cond_signal_n(cond, 1);
}
FASTCOND_API(int)
fastcond_cond_broadcast(fastcond_cond_t *cond)
{
    return _fastcond_cond_signal_n(cond, -1);
}
