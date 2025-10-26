/* Copyright (c) 2017-2025 Kristján Valur Jónsson */

#include "gil.h"
#include <assert.h>

// GIL implementation mode control
// Three modes available for experimental comparison:
//   NAIVE: Simple mutex acquire/release - no condition variables or fairness
//   UNFAIR: Uses condition variables but disables fairness mechanism
//   FAIR: Full implementation with anti-greedy fairness mechanism (default)

#ifndef FASTCOND_GIL_MODE_NAIVE
#define FASTCOND_GIL_MODE_NAIVE 0
#endif

// Control fairness behavior separately for yield() and acquire()
// yield() should generally be fair to give other threads a chance
#ifndef FASTCOND_GIL_YIELD_FAIR
#define FASTCOND_GIL_YIELD_FAIR 1
#endif

// acquire() can be greedy for performance - when enabled, acquire() doesn't check fairness
#ifndef FASTCOND_GIL_ACQUIRE_GREEDY
#define FASTCOND_GIL_ACQUIRE_GREEDY 1
#endif

// Validate mode configuration - naive mode overrides fairness settings
#if FASTCOND_GIL_MODE_NAIVE && (FASTCOND_GIL_YIELD_FAIR || !FASTCOND_GIL_ACQUIRE_GREEDY)
#error                                                                                             \
    "NAIVE mode requires unfair behavior (set FASTCOND_GIL_YIELD_FAIR=0 and FASTCOND_GIL_ACQUIRE_GREEDY=1)"
#endif

void fastcond_gil_init(struct fastcond_gil *gil)
{
    // STEP-BY-STEP ISOLATION: Add back one field at a time
    NATIVE_MUTEX_INIT(&gil->mutex);
    gil->held = 0;
    gil->n_waiting = 0;

    // TEMPORARILY DISABLED TO ISOLATE HANG
    // gil->last_owner = NATIVE_THREAD_SELF();
}

void fastcond_gil_destroy(struct fastcond_gil *gil)
{
    // Always destroy condition variables (even if NAIVE mode didn't use them)
#if FASTCOND_GIL_USE_NATIVE_COND
    NATIVE_COND_DESTROY(&gil->cond);
#else
    fastcond_cond_fini(&gil->cond);
#endif
    NATIVE_MUTEX_DESTROY(&gil->mutex);
}

// Implement the GIL logic.  A Thread can acquire the gil if
// A) the gil is not currently held and:
//   1) no one is waiting or
//   2) (fairness enabled) someone is waiting, but the last owner is not the current thread
//   3) (fairness disabled) behaves like a regular mutex - any thread can acquire

void fastcond_gil_acquire(struct fastcond_gil *gil)
{
#if FASTCOND_GIL_MODE_NAIVE
    // NAIVE mode: Simple mutex lock - no condition variables or state tracking
    // This provides the absolute minimal baseline for comparison
    NATIVE_MUTEX_LOCK(&gil->mutex);
    // In naive mode, mutex lock provides all synchronization
    // No state tracking, no condition variables
#else
    // UNFAIR and FAIR modes: Identical except for the while condition
    // Always get thread ID for state tracking (even in UNFAIR mode)
    native_thread_t self = NATIVE_THREAD_SELF();
    NATIVE_MUTEX_LOCK(&gil->mutex);

    // Fairness control: use ACQUIRE_GREEDY setting
#if FASTCOND_GIL_ACQUIRE_GREEDY
    // GREEDY mode: only wait if GIL is held (ignores fairness condition)
    while (gil->held) {
#else
    // FAIR mode: also prevent re-acquisition when others are waiting
    while (gil->held || (gil->n_waiting > 0 && NATIVE_THREAD_EQUAL(gil->last_owner, self))) {
#endif
        gil->n_waiting++;
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_WAIT(&gil->cond, &gil->mutex);
#else
        fastcond_cond_wait(&gil->cond, &gil->mutex);
#endif
        gil->n_waiting--;
    }

    assert(!gil->held);
    // Always update state tracking (even in UNFAIR mode)
    gil->last_owner = self;
    gil->held = 1;
    NATIVE_MUTEX_UNLOCK(&gil->mutex);
#endif
}

void fastcond_gil_release(struct fastcond_gil *gil)
{
#if FASTCOND_GIL_MODE_NAIVE
    // NAIVE mode: Simple mutex unlock - no state tracking or signaling
    NATIVE_MUTEX_UNLOCK(&gil->mutex);
#else
    // UNFAIR and FAIR modes: Identical behavior
    NATIVE_MUTEX_LOCK(&gil->mutex);
    assert(gil->held);

    if (gil->n_waiting > 0) {
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_SIGNAL(&gil->cond);
#else
        fastcond_cond_signal(&gil->cond);
#endif
    }
    gil->held = 0;
    NATIVE_MUTEX_UNLOCK(&gil->mutex);
#endif
}

// Yield the GIL to allow other threads to run.
// This is equivalent to release() followed immediately by acquire(),
// but provides a cleaner API for the common case of voluntarily yielding
// CPU time while maintaining lock ownership. Useful for:
// - Long-running computations that want to be cooperative
// - Implementing periodic yields in interpreter loops
// - Reducing latency spikes in real-time applications
//
// IMPLEMENTATION VARIANTS:
// - NAIVE mode: Simple mutex unlock + lock (no optimization possible)
// - FAIR/UNFAIR modes: Optimized to eliminate redundant mutex unlock/lock
//   pair between release and acquire phases, reducing mutex operations by 50%

void fastcond_gil_yield(struct fastcond_gil *gil)
{
#if FASTCOND_GIL_MODE_NAIVE
    // NAIVE mode: Simple release + acquire with mutex operations
    // No optimization possible since we just have a plain mutex
    NATIVE_MUTEX_UNLOCK(&gil->mutex);
    NATIVE_MUTEX_LOCK(&gil->mutex);
#else
    // OPTIMIZED IMPLEMENTATION: Combine release + acquire with shared mutex lock

    // Get thread ID for fairness checking (same as acquire)
    native_thread_t self = NATIVE_THREAD_SELF();

    // Single mutex lock for entire yield operation
    NATIVE_MUTEX_LOCK(&gil->mutex);

    // RELEASE PHASE: Same logic as fastcond_gil_release() but no mutex unlock
    assert(gil->held);

    if (gil->n_waiting > 0) {
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_SIGNAL(&gil->cond);
#else
        fastcond_cond_signal(&gil->cond);
#endif
    }
    gil->held = 0;

    // YIELD POINT: Other threads can now compete for the GIL
    // We remain in the mutex-protected critical section but give up GIL ownership

    // ACQUIRE PHASE: Same logic as fastcond_gil_acquire() but no mutex lock
    // The critical difference: we already hold the mutex from the release phase
    // NOTE: yield() uses its own fairness setting, independent of acquire()

#if !FASTCOND_GIL_YIELD_FAIR
    // Unfair yield: only wait if GIL is held (ignores fairness condition)
    while (gil->held) {
#else
    // Fair yield: also prevent re-acquisition when others are waiting (default)
    while (gil->held || (gil->n_waiting > 0 && NATIVE_THREAD_EQUAL(gil->last_owner, self))) {
#endif
        gil->n_waiting++;
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_WAIT(&gil->cond, &gil->mutex);
#else
        fastcond_cond_wait(&gil->cond, &gil->mutex);
#endif
        gil->n_waiting--;
    }

    assert(!gil->held);
    // Update state tracking for new acquisition
    gil->last_owner = self;
    gil->held = 1;

    // Single mutex unlock for entire yield operation
    NATIVE_MUTEX_UNLOCK(&gil->mutex);
#endif
}
