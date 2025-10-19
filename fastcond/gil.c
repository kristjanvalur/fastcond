
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

#ifndef FASTCOND_GIL_DISABLE_FAIRNESS
#define FASTCOND_GIL_DISABLE_FAIRNESS 0
#endif

// Validate mode configuration - naive mode overrides fairness settings
#if FASTCOND_GIL_MODE_NAIVE && !FASTCOND_GIL_DISABLE_FAIRNESS
#error "NAIVE mode requires fairness to be disabled (set FASTCOND_GIL_DISABLE_FAIRNESS=1)"
#endif

void fastcond_gil_init(struct fastcond_gil *gil)
{
    // Always initialize condition variables (even if NAIVE mode won't use them)
#if FASTCOND_GIL_USE_NATIVE_COND
    NATIVE_COND_INIT(gil->cond);
#else
    fastcond_cond_init(&gil->cond, NULL);
#endif

    NATIVE_MUTEX_INIT(gil->mutex);
    
    // Always initialize tracking variables (minimal overhead)
    gil->held = 0;
    gil->n_waiting = 0;
    // we can initialize this to self, fairness check will not
    // be made in the first acquisition by a thread because
    // n_waiting is 0
    gil->last_owner = NATIVE_THREAD_SELF();
}

void fastcond_gil_destroy(struct fastcond_gil *gil)
{
    // Always destroy condition variables (even if NAIVE mode didn't use them)
#if FASTCOND_GIL_USE_NATIVE_COND
    NATIVE_COND_DESTROY(gil->cond);
#else
    fastcond_cond_fini(&gil->cond);
#endif
    NATIVE_MUTEX_DESTROY(gil->mutex);
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
    NATIVE_MUTEX_LOCK(gil->mutex);
    // In naive mode, mutex lock provides all synchronization
    // No state tracking, no condition variables
#else
    // UNFAIR and FAIR modes: Identical except for the while condition
    // Always get thread ID for state tracking (even in UNFAIR mode)
    native_thread_t self = NATIVE_THREAD_SELF();
    NATIVE_MUTEX_LOCK(gil->mutex);

    // The ONLY difference between FAIR and UNFAIR modes:
#if FASTCOND_GIL_DISABLE_FAIRNESS
    // UNFAIR mode: only wait if GIL is held (ignores fairness condition)
    while (gil->held) {
#else
    // FAIR mode: also prevent re-acquisition when others are waiting
    while (gil->held || (gil->n_waiting > 0 && NATIVE_THREAD_EQUAL(gil->last_owner, self))) {
#endif
        gil->n_waiting++;
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_WAIT(gil->cond, gil->mutex);
#else
        fastcond_cond_wait(&gil->cond, &gil->mutex);
#endif
        gil->n_waiting--;
    }

    assert(!gil->held);
    // Always update state tracking (even in UNFAIR mode)
    gil->last_owner = self;
    gil->held = 1;
    NATIVE_MUTEX_UNLOCK(gil->mutex);
#endif
}

void fastcond_gil_release(struct fastcond_gil *gil)
{
#if FASTCOND_GIL_MODE_NAIVE
    // NAIVE mode: Simple mutex unlock - no state tracking or signaling
    NATIVE_MUTEX_UNLOCK(gil->mutex);
#else
    // UNFAIR and FAIR modes: Identical behavior
    NATIVE_MUTEX_LOCK(gil->mutex);
    assert(gil->held);

    if (gil->n_waiting > 0) {
#if FASTCOND_GIL_USE_NATIVE_COND
        NATIVE_COND_SIGNAL(gil->cond);
#else
        fastcond_cond_signal(&gil->cond);
#endif
    }
    gil->held = 0;
    NATIVE_MUTEX_UNLOCK(gil->mutex);
#endif
}
