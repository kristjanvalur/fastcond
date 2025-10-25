/* Copyright (c) 2017 Kristján Valur Jónsson */

#ifndef _NATIVE_PRIMITIVES_H_
#define _NATIVE_PRIMITIVES_H_

/*
 * Platform abstraction for native synchronization primitives
 * This header defines macros for the "native" condition variables,
 * mutexes, and thread IDs that should be used when not using fastcond.
 */

/* Platform detection - same as fastcond.h */
#if defined(_WIN32) || defined(_WIN64)
#define NATIVE_USE_WINDOWS 1
#include <windows.h>
#include <process.h>  /* for _beginthreadex */
#elif defined(__APPLE__) || defined(__MACH__)
#include <pthread.h>
#include <dispatch/dispatch.h>
#define NATIVE_USE_GCD 1
#else
#include <pthread.h>
#define NATIVE_USE_POSIX 1
#endif

/*
 * Thread ID abstraction
 * Windows: Use GetCurrentThreadId() which returns DWORD
 * POSIX: Use pthread_t with pthread_equal for comparison
 */
#ifdef NATIVE_USE_WINDOWS
typedef DWORD native_thread_t;
#define NATIVE_THREAD_SELF() GetCurrentThreadId()
#define NATIVE_THREAD_EQUAL(t1, t2) ((t1) == (t2))
#else
typedef pthread_t native_thread_t;
#define NATIVE_THREAD_SELF() pthread_self()
#define NATIVE_THREAD_EQUAL(t1, t2) pthread_equal((t1), (t2))
#endif

/*
 * Mutex abstraction
 * Windows: Use CRITICAL_SECTION (faster than MUTEX on Windows)
 * POSIX: Use pthread_mutex_t
 * Note: GCD doesn't have a direct mutex - uses dispatch_queue serialization,
 *       but pthread_mutex_t works fine on macOS
 */
#ifdef NATIVE_USE_WINDOWS
typedef CRITICAL_SECTION native_mutex_t;
#define NATIVE_MUTEX_INIT(mutex) InitializeCriticalSection(&(mutex))
#define NATIVE_MUTEX_DESTROY(mutex) DeleteCriticalSection(&(mutex))
#define NATIVE_MUTEX_LOCK(mutex) EnterCriticalSection(mutex)
#define NATIVE_MUTEX_UNLOCK(mutex) LeaveCriticalSection(mutex)
#else
typedef pthread_mutex_t native_mutex_t;
#define NATIVE_MUTEX_INIT(mutex) pthread_mutex_init(&(mutex), NULL)
#define NATIVE_MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
#define NATIVE_MUTEX_LOCK(mutex) pthread_mutex_lock(mutex)
#define NATIVE_MUTEX_UNLOCK(mutex) pthread_mutex_unlock(mutex)
#endif

/*
 * Condition variable abstraction
 * Windows: Use native CONDITION_VARIABLE (Vista+)
 * macOS: Use pthread_cond_t (GCD doesn't have condition variables)
 * Linux/POSIX: Use pthread_cond_t
 *
 * Note: Windows CONDITION_VARIABLE was introduced in Vista/Server 2008.
 * For older Windows versions, pthread-win32 would be needed.
 * Even though macOS has deprecated POSIX semaphores (which fastcond works
 * around using GCD semaphores), pthread condition variables are still fully
 * supported and remain the recommended approach for "native" condition variables.
 * The fastcond library uses GCD semaphores internally but provides its own
 * condition variable API on top.
 */
#ifdef NATIVE_USE_WINDOWS
/* Windows: Use native CONDITION_VARIABLE with CRITICAL_SECTION */
typedef CONDITION_VARIABLE native_cond_t;
#define NATIVE_COND_INIT(cond) InitializeConditionVariable(&(cond))
#define NATIVE_COND_DESTROY(cond) ((void)0)  /* No cleanup needed for CONDITION_VARIABLE */
#define NATIVE_COND_WAIT(cond, mutex) SleepConditionVariableCS(&(cond), &(mutex), INFINITE)
#define NATIVE_COND_SIGNAL(cond) WakeConditionVariable(&(cond))
#define NATIVE_COND_BROADCAST(cond) WakeAllConditionVariable(&(cond))
#elif defined(NATIVE_USE_GCD)
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