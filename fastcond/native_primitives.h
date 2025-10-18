/* Copyright (c) 2017 Kristján Valur Jónsson */

#ifndef _NATIVE_PRIMITIVES_H_
#define _NATIVE_PRIMITIVES_H_

/*
 * Platform abstraction for native synchronization primitives
 * This header defines macros for the "native" condition variables,
 * mutexes, and thread IDs that should be used when not using fastcond.
 */

#include <pthread.h>

/* Platform detection - same as fastcond.h */
#if defined(__APPLE__) || defined(__MACH__)
#include <dispatch/dispatch.h>
#define NATIVE_USE_GCD 1
#else
#define NATIVE_USE_POSIX 1
#endif

/*
 * Thread ID abstraction
 * All platforms use pthread_t, but we abstract the comparison
 */
typedef pthread_t native_thread_t;
#define NATIVE_THREAD_SELF() pthread_self()
#define NATIVE_THREAD_EQUAL(t1, t2) pthread_equal((t1), (t2))

/*
 * Mutex abstraction
 * All platforms use pthread_mutex_t for now
 * (GCD doesn't have a direct mutex equivalent - uses dispatch_queue serialization)
 */
typedef pthread_mutex_t native_mutex_t;
#define NATIVE_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
#define NATIVE_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
#define NATIVE_MUTEX_LOCK(mutex) pthread_mutex_lock(&(mutex))
#define NATIVE_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(&(mutex))

/*
 * Condition variable abstraction
 * macOS: Use pthread_cond_t (GCD doesn't have direct condition variables)
 * Linux/POSIX: Use pthread_cond_t
 *
 * Note: Even though macOS has deprecated POSIX semaphores (which fastcond works
 * around using GCD semaphores), pthread condition variables are still fully
 * supported and remain the recommended approach for "native" condition variables.
 * The fastcond library uses GCD semaphores internally but provides its own
 * condition variable API on top.
 */
#ifdef NATIVE_USE_GCD
/* macOS: Use pthread condition variables (GCD doesn't have them) */
typedef pthread_cond_t native_cond_t;
#define NATIVE_COND_INIT(cond) pthread_cond_init(&(cond), NULL)
#define NATIVE_COND_DESTROY(cond) pthread_cond_destroy(&(cond))
#define NATIVE_COND_WAIT(cond, mutex) pthread_cond_wait(&(cond), &(mutex))
#define NATIVE_COND_SIGNAL(cond) pthread_cond_signal(&(cond))
#define NATIVE_COND_BROADCAST(cond) pthread_cond_broadcast(&(cond))
#else
/* Linux/POSIX: Use pthread condition variables */
typedef pthread_cond_t native_cond_t;
#define NATIVE_COND_INIT(cond) pthread_cond_init(&(cond), NULL)
#define NATIVE_COND_DESTROY(cond) pthread_cond_destroy(&(cond))
#define NATIVE_COND_WAIT(cond, mutex) pthread_cond_wait(&(cond), &(mutex))
#define NATIVE_COND_SIGNAL(cond) pthread_cond_signal(&(cond))
#define NATIVE_COND_BROADCAST(cond) pthread_cond_broadcast(&(cond))
#endif

#endif /* ! defined _NATIVE_PRIMITIVES_H_ */