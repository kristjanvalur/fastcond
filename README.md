# The Fastcond project

[![CI](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml)
[![Benchmarks](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Performance](https://img.shields.io/badge/performance-38%25_faster-brightgreen)](https://kristjanvalur.github.io/fastcond/)

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

# Run performance benchmarks
./scripts/run_benchmarks.sh
```

See [BUILD.md](BUILD.md) for detailed build instructions and [dev-docs/benchmarking.md](dev-docs/benchmarking.md) for performance testing.

## Development

Before committing, format your code:
```bash
./scripts/format.sh
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines.

## Usage

They can be used instead of regular `pthread_cond_t` objects subject to the
following restrictions:

* They don't obey any non-default `pthread_condattr_t` attributes
* They don't provide *cancellation points*.
* They cannot be initialized with a static initializer

**Platform Support:**
- ✅ Linux (fully supported - tested on Ubuntu with gcc and clang)
- ⚠️ macOS (supported but without `*_timedwait` functions - macOS lacks `sem_timedwait`)
- ❓ Other POSIX systems with full semaphore support should work

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

## Performance

On multi-core systems, fastcond shows significant performance improvements over pthread:

- **38% higher throughput** in multi-threaded producer-consumer scenarios
- **26% lower latency** for message passing
- Consistent improvements across different workloads

**📊 [View Interactive Performance Results](https://kristjanvalur.github.io/fastcond/)**

See [docs/performance-analysis.md](docs/performance-analysis.md) for detailed benchmark results and analysis.

## Documentation

- **[docs/](docs/)** - User-facing documentation (performance results, API reference)
- **[dev-docs/](dev-docs/)** - Development documentation (benchmarking, CI/CD, deployment)
- **[BUILD.md](BUILD.md)** - Build instructions
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines
