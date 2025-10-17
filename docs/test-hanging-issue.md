# Test Hanging Issue - Resolution

## Problem

The test suite was hanging when running all tests. Specifically, the `strongtest_weak_smoke` test would hang indefinitely.

## Root Cause

**The `strongtest` program specifically validates strong condition variable semantics** - namely that only threads that are already waiting get woken up (no wakeup stealing).

From `test/strongtest.c`:
```c
/*
 * Similar to the qtest, this version has only one condition variable, and
 * a single producer and server. It tests how a `strong` condition variable wakes up 
 * at least one of the threads already waiting, i.e. that the wakeup cannot be
 * stolen by the other thread.
 */
```

The test has:
- **One sender thread** (producer)
- **One receiver thread** (consumer)  
- **One shared condition variable** for both

These threads are **not interchangeable** - a sender signal should wake the receiver, and vice versa.

**The weak condition variable allows wakeup stealing**, meaning a thread that hasn't started waiting yet can "steal" a wakeup intended for another thread. In strongtest's scenario:

1. Sender fills queue, signals condition, and immediately waits (queue full)
2. The signal might wake the sender itself instead of the receiver (wakeup stealing)
3. Deadlock: sender waits for receiver, but receiver never got woken

## Solution

**Don't test `strongtest` with the weak variant** - this is expected behavior and documented.

### Changes Made

#### 1. CMakeLists.txt
Removed the `strongtest_weak_smoke` test and added explanation:
```cmake
# NOTE: strongtest_weak is NOT tested because strongtest specifically
# validates strong semantics (no wakeup stealing). The weak variant
# allows wakeup stealing and will deadlock in this test scenario.
# This is expected behavior - weak variant should only be used when
# all waiting threads are interchangeable.
```

#### 2. scripts/run_tests.sh
Removed execution of `strongtest_wcond` with explanation comment.

#### 3. BUILD.md
Added note explaining why strongtest_wcond is not run.

#### 4. .github/copilot-instructions.md  
Updated to explicitly mention that strongtest_wcond will deadlock.

## Verification

After fixes:
```bash
$ ctest --test-dir build --output-on-failure
Test project /home/kristjan/git/fastcond/build
    Start 1: qtest_pthread_smoke
1/5 Test #1: qtest_pthread_smoke ..............   Passed    0.00 sec
    Start 2: qtest_fastcond_smoke
2/5 Test #2: qtest_fastcond_smoke .............   Passed    0.00 sec
    Start 3: qtest_weak_smoke
3/5 Test #3: qtest_weak_smoke .................   Passed    0.00 sec
    Start 4: strongtest_pthread_smoke
4/5 Test #4: strongtest_pthread_smoke .........   Passed    0.00 sec
    Start 5: strongtest_fastcond_smoke
5/5 Test #5: strongtest_fastcond_smoke ........   Passed    0.00 sec

100% tests passed, 0 tests failed out of 5
```

All tests now pass without hanging!

## When to Use Weak vs Strong

**Use Weak Condition Variable When:**
- All waiting threads are functionally equivalent
- Any thread can handle any wakeup
- Example: Worker thread pool where all workers do the same thing

**Use Strong Condition Variable When:**
- Threads have different roles (producer vs consumer)
- Specific threads need specific wakeups
- Multiple condition variables would normally be used
- Example: Single condition variable coordinating different thread types

**Never Use Weak Condition Variable:**
- With different thread roles sharing one condition variable
- In strongtest-like scenarios

## Key Takeaway

This is **not a bug** - it's the expected behavior demonstrating the limitation of weak condition variables. The test suite now correctly reflects this by testing weak variants only where appropriate (qtest with separate conditions for producer/consumer).
