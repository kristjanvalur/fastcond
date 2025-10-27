# The Fastcond project

[![CI](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml)
[![Benchmarks](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Performance](https://img.shields.io/badge/performance-38%25_faster-brightgreen)](https://kristjanvalur.github.io/fastcond/)

> **❄️ 38% faster than pthread condition variables** — Because life's too short for sluggish synchronisation

Fast, reliable POSIX condition variable alternatives forged during Iceland's long winter nights. When `pthread_cond_t` keeps you debugging instead of stargazing, fastcond delivers the performance that lets you spend more time on life's finer pleasures.

## 📈 Performance That Speaks for Itself

| Metric | pthread | **fastcond** | Improvement |
|--------|---------|-------------|-------------|
| **Throughput** | 403K items/s | **559K items/s** | **🔥 38% faster** |
| **Average Latency** | 11.9 μs | **8.8 μs** | **⚡ 26% lower** |
| **Tail Latency (max)** | 149 μs | **111 μs** | **📉 25% better** |

**🎯 [See Live Performance Dashboard →](https://kristjanvalur.github.io/fastcond/)**

**� [Cross-Platform Performance Comparison →](https://kristjanvalur.github.io/fastcond/performance/)**

**�🎭 [Explore GIL Fairness Analysis →](https://kristjanvalur.github.io/fastcond/gil-fairness.html)**

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

## How It Works

**FastCond is a user-space condition variable implementation using only semaphores.**

Unlike `pthread_cond_t` which typically relies on kernel primitives like futex (Linux) or Mach semaphores (macOS), fastcond implements full POSIX condition variable semantics entirely in user space. This provides:

- **Portability**: Works on any platform with semaphore support
- **Transparency**: Full control over wakeup behavior and scheduling
- **Performance**: On macOS, direct GCD dispatch semaphores avoid pthread compatibility overhead

### The Wakeup Stealing Problem

The central challenge: when `signal()` is called with N threads waiting, those *specific* threads must wake—but semaphores don't track identity. A naive implementation allows "wakeup stealing":

```
Thread A waiting → Thread B signals (posts semaphore) → Thread C (new arrival) steals wakeup → Thread A stuck forever ❌
```

### The Spurious Wakeup Solution

FastCond prevents stealing with a clever mechanism: when a new thread arrives and sees pending wakeups (`n_wakeup > 0`), instead of entering the semaphore wait, it:

1. Takes a **spurious wakeup** (unlock → yield → relock → return immediately)
2. Caller's `while (!condition)` loop retries
3. By next iteration, original threads have consumed their wakeups
4. New thread can now wait properly without stealing

This uses POSIX's *allowed* spurious wakeups as an *enforcement mechanism* for strong semantics.

### Three-Counter Architecture

```c
volatile int w_waiting;  // Threads blocked on semaphore RIGHT NOW
volatile int n_waiting;  // Threads anywhere in wait() call
volatile int n_wakeup;   // Outstanding wakeup obligations
```

**Why three?** Timing asymmetry: `w_waiting` decrements *during* the wait (after sem_wait, before mutex relock), while `n_waiting` spans the *entire* operation. The invariant `n_wakeup ≤ n_waiting` requires tracking both separately.

**The `MAYBE_YIELD()` optimization**: Yielding CPU when taking spurious wakeup gives original threads time to consume their wakeups, reducing retry frequency from dozens to typically 1-2 iterations.

For the complete algorithm explanation with references to Birrell's semaphore paper and detailed timing analysis, see the comprehensive documentation in `fastcond/fastcond.c`.

## Usage

They can be used instead of regular `pthread_cond_t` objects subject to the
following restrictions:

* They don't obey any non-default `pthread_condattr_t` attributes
* They don't provide *cancellation points*.
* They cannot be initialized with a static initializer

## Why Choose Fastcond? 

Because life's rather too short for sluggish condition variables, wouldn't you say? 🐌→🚀

**The Numbers Tell the Tale:**
- **38% higher throughput** — Your producer-consumer patterns will be positively chuffed
- **26% lower latency** — Messages fairly zip through the system
- **25% better tail latency** — Even your worst-case scenarios perform rather well
- **Consistent improvements** — Not merely cherry-picked benchmarks, proper improvements across workloads

> **🍎 macOS Performance Highlight:** Particularly striking improvements on macOS, where fastcond's direct use of GCD dispatch semaphores bypasses the pthread compatibility layer. While pthread condition variables on macOS go through translation overhead, fastcond speaks the platform's native tongue. Rather like the difference between a fluent local versus going through a translator! See the [cross-platform comparison](https://kristjanvalur.github.io/fastcond/performance/) for detailed metrics.

**Platform Compatibility:**
- ✅ **Linux** - Thoroughly tested and production-ready
- ✅ **macOS** - Excellent performance with GCD semaphores (missing `*_timedwait` due to platform limitations)
- ✅ **Windows** - Full support with native CONDITION_VARIABLE implementation (v0.2.0+)
- ❓ **Other POSIX** - Should work perfectly well if you have proper semaphore support

**� [View Cross-Platform Performance Comparison →](https://kristjanvalur.github.io/fastcond/performance/)**

**�🎯 [Explore the Interactive Performance Dashboard →](https://kristjanvalur.github.io/fastcond/)**

*Featuring benchmarks crafted during Nordic winter nights, proper charts, and all the performance data needed to free you for poetry and cocoa.*

## Documentation

- **[docs/](docs/)** - User-facing documentation (performance results, API reference)
- **[dev-docs/](dev-docs/)** - Development documentation (benchmarking, CI/CD, deployment)
- **[BUILD.md](BUILD.md)** - Build instructions
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines
