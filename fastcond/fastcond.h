/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#ifndef _FASTCOND_H_
#define _FASTCOND_H_

/* Version information */
#define FASTCOND_VERSION_MAJOR 0
#define FASTCOND_VERSION_MINOR 1
#define FASTCOND_VERSION_PATCH 0

/* Semantic version string */
#define FASTCOND_VERSION "0.1.0"

/* Numeric version for compile-time comparisons: MAJOR*10000 + MINOR*100 + PATCH */
#define FASTCOND_VERSION_NUM (FASTCOND_VERSION_MAJOR * 10000 + \
                              FASTCOND_VERSION_MINOR * 100 + \
                              FASTCOND_VERSION_PATCH)

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

/* The _weak_ condition variable.  See fastcond.c for details */

typedef struct _fastcond_wcond_t {
#ifdef FASTCOND_USE_WINDOWS
    HANDLE sem;  /* Windows semaphore handle */
#elif defined(FASTCOND_USE_GCD)
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
    volatile int waiting; /* to allow PyCOND_SIGNAL to be a no-op */
} fastcond_wcond_t;

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

/* the _strong_ condition variable.  See fastcond.c for details */

typedef struct _fastcond_cond_t {
    fastcond_wcond_t wait;  /* the inner weak condition variable */
    volatile int n_waiting; /* number of threads in 'wait' */
    volatile int n_wakeup;  /* number of awoken threads in 'wait' that haven't exited yet. */
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

#endif /* ! defined _FASTCOND_H_ */
