# The Fastcond project

This provides an alternate implementation of posix _condition variables_,
based only on posix semaphores.

## Quick Start

```bash
# Build with CMake (recommended)
cmake -B build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
# or
./scripts/run_tests.sh

# Run benchmarks
./scripts/benchmark.sh
```

See [BUILD.md](BUILD.md) for detailed build instructions.

They can be used instead of regular `pthread_cond_t` objects subject to the
following restrictions:

* They don't obey any non-default `pthread_condattr_t` attributes
* They don't provide *cancellation points*.
* They cannot be initialized with a static initializer

 Two versions are provided, the _weak_ and the _strong_ condition variables.  This
 terminology is my own.

 - The _weak_ version relaxes one of the classic condition variable promises, namely
 that _one of the threads already waiting_ gets awoken by a `pthread_cond_signal()`
 (or all of them in case of a `pthread_cond_broadcast()`), and instead, guarantees
 only that _a_ thread will wake up. I.e. it is possible that a thread which has not
 yet started to wait, will be the one woken up.

 - At the cost of some slight added complexity, the _strong_ version will uphold the
 aforementioned promise.

 Interestingly, tests on multicore machines show that the both versions outperform
 the regular `pthread_cond_t` primitives, and the _strong_ type results in much lower
 latency.

 This could be used as a drop-in replacement in many multithreaded programs where
 latency is of prime importance.
