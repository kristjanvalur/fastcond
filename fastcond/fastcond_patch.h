/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#ifndef _FASTCOND_PATCH_H_
#define _FASTCOND_PATCH_H_

#include "fastcond.h"

/*
 * Handy macros to patch your programs to override the regular
 * condition variable stuff with fastcond implementations.
 *
 * Supports both POSIX pthread and Windows CONDITION_VARIABLE APIs.
 *
 * Usage:
 *   #define FASTCOND_PATCH_COND  // or FASTCOND_PATCH_WCOND
 *   #include "fastcond_patch.h"
 *
 * Platform detection:
 *   - Windows: Patches CONDITION_VARIABLE/InitializeConditionVariable/etc.
 *   - POSIX: Patches pthread_cond_t/pthread_cond_init/etc.
 *
 * Limitations:
 *   - No static initializer support (PTHREAD_COND_INITIALIZER, CONDITION_VARIABLE_INIT)
 *   - Windows: No SleepConditionVariableSRW support (only SleepConditionVariableCS)
 *   - POSIX: No support for non-default pthread_condattr_t attributes
 *   - No cancellation points
 */

/* Platform detection */
#if defined(_WIN32) || defined(_WIN64)
#define FASTCOND_PATCH_WINDOWS 1
#endif

/*
 * Windows API wrappers
 * Windows SleepConditionVariableCS has different signature, needs wrappers
 */
#ifdef FASTCOND_PATCH_WINDOWS

/* Wrapper for SleepConditionVariableCS that matches fastcond signature */
static inline int fastcond_patch_sleep_cs(void *cond, CRITICAL_SECTION *cs,
                                          int (*wait_fn)(void *, native_mutex_t *)) {
    return wait_fn((void *)cond, (native_mutex_t *)cs) ? 1 : 0;
}

/* Define wrapper macros that will be used in the patch definitions */
#define FASTCOND_PATCH_SLEEP_WCOND(cond, cs, timeout)                                             \
    fastcond_patch_sleep_cs((cond), (cs), (int (*)(void *, native_mutex_t *))fastcond_wcond_wait)

#define FASTCOND_PATCH_SLEEP_COND(cond, cs, timeout)                                              \
    fastcond_patch_sleep_cs((cond), (cs), (int (*)(void *, native_mutex_t *))fastcond_cond_wait)

#endif /* FASTCOND_PATCH_WINDOWS */

#if defined FASTCOND_PATCH_WCOND
/* use the weak condition variable.  This is sufficient for simple
 * synchronization, but makes no promises about _which_ threads get awoken.
 */
#ifdef FASTCOND_PATCH_WINDOWS
/* Windows CONDITION_VARIABLE API */
#define CONDITION_VARIABLE fastcond_wcond_t
#define InitializeConditionVariable(cond) fastcond_wcond_init((cond), NULL)
#define SleepConditionVariableCS(cond, cs, timeout) FASTCOND_PATCH_SLEEP_WCOND((cond), (cs), (timeout))
#define WakeConditionVariable(cond) fastcond_wcond_signal(cond)
#define WakeAllConditionVariable(cond) fastcond_wcond_broadcast(cond)
/* Note: Windows CONDITION_VARIABLE has no destroy function, but fastcond does */
#define DeleteConditionVariable(cond) fastcond_wcond_fini(cond) /* Non-standard, but useful */
#else
/* POSIX pthread API */
#define pthread_cond_t fastcond_wcond_t
#define pthread_cond_init fastcond_wcond_init
#define pthread_cond_fini fastcond_wcond_fini
#define pthread_cond_destroy fastcond_wcond_fini
#define pthread_cond_wait fastcond_wcond_wait
#define pthread_cond_timedwait fastcond_wcond_timedwait
#define pthread_cond_signal fastcond_wcond_signal
#define pthread_cond_broadcast fastcond_wcond_broadcast
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER not_implemented
#endif

#elif defined FASTCOND_PATCH_COND
/* use the _strong_ condition variable, the one that only wakes up those
 * threads already waiting
 */
#ifdef FASTCOND_PATCH_WINDOWS
/* Windows CONDITION_VARIABLE API */
#define CONDITION_VARIABLE fastcond_cond_t
#define InitializeConditionVariable(cond) fastcond_cond_init((cond), NULL)
#define SleepConditionVariableCS(cond, cs, timeout) FASTCOND_PATCH_SLEEP_COND((cond), (cs), (timeout))
#define WakeConditionVariable(cond) fastcond_cond_signal(cond)
#define WakeAllConditionVariable(cond) fastcond_cond_broadcast(cond)
/* Note: Windows CONDITION_VARIABLE has no destroy function, but fastcond does */
#define DeleteConditionVariable(cond) fastcond_cond_fini(cond) /* Non-standard, but useful */
#else
/* POSIX pthread API */
#define pthread_cond_t fastcond_cond_t
#define pthread_cond_init fastcond_cond_init
#define pthread_cond_fini fastcond_cond_fini
#define pthread_cond_destroy fastcond_cond_fini
#define pthread_cond_wait fastcond_cond_wait
#define pthread_cond_timedwait fastcond_cond_timedwait
#define pthread_cond_signal fastcond_cond_signal
#define pthread_cond_broadcast fastcond_cond_broadcast
#undef PTHREAD_COND_INITIALIZER
#define PTHREAD_COND_INITIALIZER not_implemented
#endif

#endif

#endif /* ! defined _FASTCOND_PATCH_H_ */
