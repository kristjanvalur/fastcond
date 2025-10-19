#include "fastcond.h"
#include "native_primitives.h"

// Conditional compilation: use fastcond or native pthread condition variables
// Define FASTCOND_GIL_USE_NATIVE_COND=1 to use native pthread conditions
#ifndef FASTCOND_GIL_USE_NATIVE_COND
#define FASTCOND_GIL_USE_NATIVE_COND 0
#endif

// FAIRNESS CONTROL CONFIGURATION:
// The GIL provides separate fairness controls for yield() and acquire() operations:
//
// FASTCOND_GIL_YIELD_FAIR (default: 1)
//   Controls whether fastcond_gil_yield() respects fairness. When enabled (default),
//   yield() will block if other threads are waiting, ensuring cooperative scheduling.
//   When disabled, yield() becomes greedy and may immediately reacquire the GIL.
//
// FASTCOND_GIL_ACQUIRE_GREEDY (default: 1)
//   Controls whether fastcond_gil_acquire() ignores fairness for performance.
//   When enabled (default), acquire() becomes greedy and doesn't check for waiting
//   threads. When disabled, acquire() respects fairness like traditional GIL behavior.
//
// This separation allows yield() to be fair (encouraging cooperation) while
// acquire() can be optimized for performance in scenarios where fairness isn't critical.

// A GIL.  This is a simple implementation of a Global Interpreter Lock (GIL) using condition
// variables. The difference from a plain mutex, is that it disallows the same thread from
// re-acquiring the lock, if there are other threads waiting for it.  Regular mutexes are greedy and
// allow the same thread to re-acquire the lock, which is likely to happen due to scheduling
// preference.  The GIL should try to be fair and allow another thread a chance to acquire it.
//
// PYTHON-LIKE USAGE PATTERN:
// The recommended usage pattern simulates realistic Python interpreter behavior:
// 1. Thread startup: fastcond_gil_acquire() - gain initial GIL ownership
// 2. Regular execution: fastcond_gil_yield() - cooperative yielding (~90% of operations)
// 3. I/O operations: fastcond_gil_release() + I/O work + fastcond_gil_acquire() (~10%)
// 4. Thread shutdown: fastcond_gil_release() - release GIL ownership
//
// This pattern provides more realistic fairness behavior compared to simple acquire/release
// cycles and better represents how Python threads interact with the GIL in practice.

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
void fastcond_gil_yield(
    struct fastcond_gil *gil); // Release and immediately reacquire (cooperative yielding)
