# Copilot Instructions for fastcond

## Project Overview

This is a specialized C library providing fast POSIX condition variable alternatives (`fastcond`) using only semaphores. Two variants exist with different semantics:

- **Weak condition variable** (`fastcond_wcond_t`): May wake threads not yet waiting—violates strict POSIX semantics but offers performance
- **Strong condition variable** (`fastcond_cond_t`): Guarantees only already-waiting threads wake up—full POSIX semantics via spurious wakeup mechanism

Both outperform `pthread_cond_t` on multicore systems, with strong variant showing significantly lower latency.

## Critical Architecture Patterns

### Layered Implementation
- Strong condition variable **wraps** weak variant (`fastcond_cond_t` contains `fastcond_wcond_t wait`)
- Weak variant directly wraps semaphores for signaling
- Strong variant adds `n_waiting` and `n_wakeup` bookkeeping to prevent wakeup stealing

### Wakeup Stealing Prevention
In `fastcond_cond_timedwait()`, if `n_wakeup > 0`, new waiters trigger spurious wakeup instead of entering wait state. This is the **key mechanism** ensuring strong semantics—see the unlock/yield/relock pattern at fastcond.c:238-248.

### Error Handling Convention
Functions return 0 on success, errno value on failure (not -1). Never set errno directly—return it:
```c
return sem_init(&cond->sem, 0, 0) ? errno : 0;
```

## Build & Test Workflow

### Building Tests
From `test/` directory:
```bash
make all  # Builds 4 variants: qtest_pt, qtest_fc, strongtest_pt, strongtest_fc
```

Build targets follow `<test>_<impl>` pattern:
- `_pt` suffix = pthread implementation (baseline)
- `_fc` suffix = fastcond implementation (uses `DOPATCH=-DFASTCOND_PATCH_$(PATCH)` macro)

Default patch mode is `COND` (strong variant). Change via `PATCH=WCOND` for weak variant.

### Running Tests
```bash
./qtest_fc 10000 4 10      # Args: data_count, num_threads, queue_size
./strongtest_fc 10000 5    # Args: data_count, queue_size
```

`qtest` uses separate not_empty/not_full conditions (producer-consumer). `strongtest` uses **single condition** for both threads—this specifically validates strong semantics (prevents wakeup stealing).

**Important**: `strongtest` with weak variant (`strongtest_wcond`) will deadlock and should not be run. This is expected—the test validates that only waiting threads wake, which weak variant cannot guarantee.

## Drop-in Replacement Pattern

`fastcond_patch.h` provides preprocessor macros to replace `pthread_cond_*`:

```c
#define FASTCOND_PATCH_COND    // For strong variant
// OR
#define FASTCOND_PATCH_WCOND   // For weak variant

#include "fastcond_patch.h"
// Now pthread_cond_t is fastcond_cond_t, etc.
```

**Limitation**: No static initializer support (`PTHREAD_COND_INITIALIZER` becomes `not_implemented`).

## Key Constraints

1. **No cancellation points**: Unlike pthread, wait functions ignore cancellation
2. **No condattr support**: Attributes parameter ignored (null check not enforced)
3. **Spurious wakeups are features**: Strong variant intentionally generates them to preserve semantics
4. **Thread equivalence matters**: Use weak variant only when all waiting threads are interchangeable

## File Organization

- `fastcond/fastcond.h`: Public API declarations for both variants
- `fastcond/fastcond.c`: Implementation with extensive algorithmic commentary
- `fastcond/fastcond_patch.h`: Macro definitions for pthread replacement
- `test/qtest.c`: Multi-condition producer-consumer test (latency measurement)
- `test/strongtest.c`: Single-condition test validating strong semantics

## Code Review Checklist

When modifying core logic:
1. Does it preserve the "wakeup stealing" prevention in strong variant?
2. Are errors returned as errno values (not via errno global)?
3. Does it maintain the invariant `n_wakeup <= n_waiting`?
4. Are mutex unlock/lock pairs properly sequenced around semaphore operations?
5. Have you tested with both `strongtest` and `qtest` against pthread baseline?
