#include "fastcond.h"
#include "native_primitives.h"

// Conditional compilation: use fastcond or native pthread condition variables
// Define FASTCOND_GIL_USE_NATIVE_COND=1 to use native pthread conditions
#ifndef FASTCOND_GIL_USE_NATIVE_COND
#define FASTCOND_GIL_USE_NATIVE_COND 0
#endif

// A GIL.  This is a simple implementation of a Global Interpreter Lock (GIL) using condition
// variables. The difference from a plain mutex, is that it disallows the same thread from
// re-acquiring the lock, if there are other threads waiting for it.  Regular mutexes are greedy and
// allow the same thread to re-acquire the lock, which is likely to happen due to scheduling
// preference.  The GIL should try to be fair and allow another thread a chance to acquire it.

struct fastcond_gil {
#if FASTCOND_GIL_USE_NATIVE_COND
    native_cond_t cond;
#else
    fastcond_cond_t cond;
#endif
    native_mutex_t mutex;
    native_thread_t last_owner;
    int held;
    int n_waiting;
};

// Function declarations
void fastcond_gil_init(struct fastcond_gil *gil);
void fastcond_gil_destroy(struct fastcond_gil *gil);
void fastcond_gil_acquire(struct fastcond_gil *gil);
void fastcond_gil_release(struct fastcond_gil *gil);
