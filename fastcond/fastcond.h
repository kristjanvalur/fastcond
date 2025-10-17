/* Copyright (c) 2017 Kristján Valur Jónsson */

#ifndef _FASTCOND_H_
#define _FASTCOND_H_

#include <pthread.h> /* for the pthread mutex that we use */

/* Platform detection and semaphore selection */
#if defined(__APPLE__) || defined(__MACH__)
#include <dispatch/dispatch.h>
#define FASTCOND_USE_GCD 1
#else
#include <semaphore.h>
#endif

#define FASTCOND_API(v) v

/* The _weak_ condition variable.  See fastcond.c for details */

typedef struct _fastcond_wcond_t {
#ifdef FASTCOND_USE_GCD
    dispatch_semaphore_t sem;
#else
    sem_t sem;
#endif
    int waiting; /* to allow PyCOND_SIGNAL to be a no-op */
} fastcond_wcond_t;

FASTCOND_API(int)
fastcond_wcond_init(fastcond_wcond_t *restrict cond, const pthread_condattr_t *restrict attr);

FASTCOND_API(int)
fastcond_wcond_fini(fastcond_wcond_t *cond);

FASTCOND_API(int)
fastcond_wcond_wait(fastcond_wcond_t *restrict cond, pthread_mutex_t *restrict mutex);

FASTCOND_API(int)
fastcond_wcond_timedwait(fastcond_wcond_t *restrict cond, pthread_mutex_t *restrict mutex,
                         const struct timespec *restrict abstime);

FASTCOND_API(int)
fastcond_wcond_signal(fastcond_wcond_t *cond);

FASTCOND_API(int)
fastcond_wcond_broadcast(fastcond_wcond_t *cond);

/* the _strong_ condition variable.  See fastcond.c for details */

typedef struct _fastcond_cond_t {
    fastcond_wcond_t wait; /* the inner weak condition variable */
    int n_waiting;         /* number of threads in 'wait' */
    int n_wakeup;          /* number of awoken threads in 'wait' that haven't exited yet. */
} fastcond_cond_t;

FASTCOND_API(int)
fastcond_cond_init(fastcond_cond_t *restrict cond, const pthread_condattr_t *restrict attr);

FASTCOND_API(int)
fastcond_cond_fini(fastcond_cond_t *cond);

FASTCOND_API(int)
fastcond_cond_wait(fastcond_cond_t *restrict cond, pthread_mutex_t *restrict mutex);

FASTCOND_API(int)
fastcond_cond_timedwait(fastcond_cond_t *restrict cond, pthread_mutex_t *restrict mutex,
                        const struct timespec *restrict abstime);

FASTCOND_API(int)
fastcond_cond_signal(fastcond_cond_t *cond);

FASTCOND_API(int)
fastcond_cond_broadcast(fastcond_cond_t *cond);

#endif /* ! defined _FASTCOND_H_ */
