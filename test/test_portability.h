/* Copyright (c) 2025 Kristján Valur Jónsson */
/*
 * Cross-platform portability layer for test programs
 *
 * Provides thin wrappers around threading and timing primitives to allow
 * test programs to compile on both POSIX (Linux/macOS) and Windows platforms.
 *
 * On Windows, this allows testing fastcond with native CONDITION_VARIABLE
 * rather than requiring pthread-win32 compatibility layer.
 */

#ifndef TEST_PORTABILITY_H
#define TEST_PORTABILITY_H

#ifdef _WIN32
#define TEST_USE_WINDOWS 1
#include <process.h> /* For _beginthread */
#include <windows.h>
#else
#define TEST_USE_WINDOWS 0
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#endif

/*
 * Thread abstraction
 * Maps to pthread_t on POSIX, HANDLE on Windows
 */
#if TEST_USE_WINDOWS
typedef HANDLE test_thread_t;
typedef unsigned(__stdcall *test_thread_func_t)(void *);
#define TEST_THREAD_FUNC_RETURN unsigned __stdcall
#define TEST_THREAD_RETURN return 0

/* Windows thread creation wrapper */
static inline int test_thread_create(test_thread_t *thread, void *attr, test_thread_func_t func,
                                     void *arg)
{
    (void) attr; /* Attributes not supported in this simple wrapper */
    *thread = (HANDLE) _beginthreadex(NULL, 0, func, arg, 0, NULL);
    return (*thread == 0) ? -1 : 0;
}

static inline int test_thread_join(test_thread_t thread, void **retval)
{
    (void) retval; /* Return value not captured in this simple wrapper */
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline test_thread_t test_thread_self(void)
{
    return GetCurrentThread();
}

static inline int test_thread_equal(test_thread_t t1, test_thread_t t2)
{
    return GetThreadId(t1) == GetThreadId(t2);
}

#else
/* POSIX */
typedef pthread_t test_thread_t;
typedef void *(*test_thread_func_t)(void *);
#define TEST_THREAD_FUNC_RETURN void *
#define TEST_THREAD_RETURN return NULL

#define test_thread_create pthread_create
#define test_thread_join pthread_join
#define test_thread_self pthread_self
#define test_thread_equal pthread_equal
#endif

/*
 * Mutex abstraction
 * Maps to pthread_mutex_t on POSIX, CRITICAL_SECTION on Windows
 */
#if TEST_USE_WINDOWS
typedef CRITICAL_SECTION test_mutex_t;

static inline int test_mutex_init(test_mutex_t *mutex, void *attr)
{
    (void) attr;
    InitializeCriticalSection(mutex);
    return 0;
}

static inline int test_mutex_destroy(test_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}

static inline int test_mutex_lock(test_mutex_t *mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

static inline int test_mutex_unlock(test_mutex_t *mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

#else
/* POSIX */
typedef pthread_mutex_t test_mutex_t;
#define test_mutex_init pthread_mutex_init
#define test_mutex_destroy pthread_mutex_destroy
#define test_mutex_lock pthread_mutex_lock
#define test_mutex_unlock pthread_mutex_unlock
#endif

/*
 * Condition variable abstraction
 * Maps to pthread_cond_t on POSIX, CONDITION_VARIABLE on Windows
 */
#if TEST_USE_WINDOWS
typedef CONDITION_VARIABLE test_cond_t;

static inline int test_cond_init(test_cond_t *cond, void *attr)
{
    (void) attr;
    InitializeConditionVariable(cond);
    return 0;
}

static inline int test_cond_destroy(test_cond_t *cond)
{
    (void) cond; /* No cleanup needed for CONDITION_VARIABLE */
    return 0;
}

static inline int test_cond_wait(test_cond_t *cond, test_mutex_t *mutex)
{
    return SleepConditionVariableCS(cond, mutex, INFINITE) ? 0 : -1;
}

static inline int test_cond_signal(test_cond_t *cond)
{
    WakeConditionVariable(cond);
    return 0;
}

static inline int test_cond_broadcast(test_cond_t *cond)
{
    WakeAllConditionVariable(cond);
    return 0;
}

#else
/* POSIX */
typedef pthread_cond_t test_cond_t;
#define test_cond_init pthread_cond_init
#define test_cond_destroy pthread_cond_destroy
#define test_cond_wait pthread_cond_wait
#define test_cond_signal pthread_cond_signal
#define test_cond_broadcast pthread_cond_broadcast
#endif

/*
 * Timing abstraction
 * Maps to clock_gettime on POSIX, QueryPerformanceCounter on Windows
 */
#if TEST_USE_WINDOWS
typedef struct {
    long tv_sec;
    long tv_nsec;
} test_timespec_t;

/* Windows high-resolution timing using QueryPerformanceCounter */
static inline int test_clock_gettime(test_timespec_t *ts)
{
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&counter);

    /* Convert to seconds and nanoseconds */
    ts->tv_sec = (long) (counter.QuadPart / frequency.QuadPart);
    ts->tv_nsec =
        (long) (((counter.QuadPart % frequency.QuadPart) * 1000000000LL) / frequency.QuadPart);

    return 0;
}

#else
/* POSIX */
typedef struct timespec test_timespec_t;

static inline int test_clock_gettime(test_timespec_t *ts)
{
    return clock_gettime(CLOCK_MONOTONIC, ts);
}
#endif

/*
 * Scheduler yield abstraction
 */
#if TEST_USE_WINDOWS
#define test_sched_yield() Sleep(0)
#else
#define test_sched_yield() sched_yield()
#endif

/*
 * Microsecond sleep abstraction
 */
#if TEST_USE_WINDOWS
static inline void test_usleep(unsigned int usec)
{
    /* Windows Sleep() takes milliseconds, with minimum resolution of 1ms
     * For sub-millisecond delays, use Sleep(0) to yield time slice
     * For >= 1ms, round up to ensure at least the requested delay
     */
    if (usec == 0) {
        return; /* No sleep requested */
    } else if (usec < 1000) {
        Sleep(0); /* Yield CPU for sub-millisecond delays */
    } else {
        /* Convert microseconds to milliseconds, rounding up */
        Sleep((usec + 999) / 1000);
    }
}
#define usleep test_usleep
#else
#include <unistd.h>
/* usleep is already available on POSIX */
#endif

/*
 * Atomic operations abstraction
 * Windows: Use InterlockedXxx intrinsics
 * POSIX: Use GCC __sync builtins
 */
#if TEST_USE_WINDOWS
#include <intrin.h>
#define __sync_fetch_and_add(ptr, value) InterlockedExchangeAdd((volatile LONG *) (ptr), (value))
#define __sync_add_and_fetch(ptr, value)                                                           \
    (InterlockedExchangeAdd((volatile LONG *) (ptr), (value)) + (value))
#define __sync_sub_and_fetch(ptr, value)                                                           \
    (InterlockedExchangeAdd((volatile LONG *) (ptr), -(value)) - (value))
#define __sync_bool_compare_and_swap(ptr, oldval, newval)                                          \
    (InterlockedCompareExchange((volatile LONG *) (ptr), (newval), (oldval)) == (oldval))
#endif

/*
 * Time function abstraction
 */
#if TEST_USE_WINDOWS
#include <time.h>
/* time() is available in time.h on Windows */
#endif

/*
 * Time difference calculation helper
 * Returns difference in seconds as a double
 */
static inline double test_timespec_diff(const test_timespec_t *end, const test_timespec_t *start)
{
    return (end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec) / 1e9;
}

#endif /* TEST_PORTABILITY_H */
