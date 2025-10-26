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
