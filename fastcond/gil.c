
#include "gil.h"
#include <assert.h>

// Fairness mechanism control: disable fairness to behave like a plain mutex
// Define FASTCOND_GIL_DISABLE_FAIRNESS=1 to disable the fairness mechanism
// When disabled, the GIL behaves like a regular mutex with greedy re-acquisition
#ifndef FASTCOND_GIL_DISABLE_FAIRNESS
#define FASTCOND_GIL_DISABLE_FAIRNESS 0
#endif

void fastcond_gil_init(struct fastcond_gil *gil)
{
#if FASTCOND_GIL_USE_NATIVE_COND
    NATIVE_COND_INIT(gil->cond);
#else
    fastcond_cond_init(&gil->cond, NULL);
#endif
    NATIVE_MUTEX_INIT(gil->mutex);
    gil->held = 0;
    gil->n_waiting = 0;
    // we can initialize this to self, fairness check will not
    // be made in the first acquisition by a thread because
    // n_waiting is 0
    gil->last_owner = NATIVE_THREAD_SELF();
}

void fastcond_gil_destroy(struct fastcond_gil *gil)
{
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
#if !FASTCOND_GIL_DISABLE_FAIRNESS
    native_thread_t self = NATIVE_THREAD_SELF();
#endif
    NATIVE_MUTEX_LOCK(gil->mutex);

    // Wait whilst:
    // - GIL is held, OR
    // - (fairness enabled) Others are waiting AND we were the last owner
#if FASTCOND_GIL_DISABLE_FAIRNESS
    // Simplified condition: only wait if GIL is held (plain mutex behavior)
    while (gil->held) {
#else
    // Full fairness condition: also check if we were the last owner
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
#if !FASTCOND_GIL_DISABLE_FAIRNESS
    gil->last_owner = self;
#endif
    gil->held = 1;
    NATIVE_MUTEX_UNLOCK(gil->mutex);
}

void fastcond_gil_release(struct fastcond_gil *gil)
{
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
}
