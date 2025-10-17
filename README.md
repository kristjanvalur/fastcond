# The Fastcond project

[![CI](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/ci.yml)
[![Benchmarks](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml/badge.svg)](https://github.com/kristjanvalur/fastcond/actions/workflows/benchmarks.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Performance](https://img.shields.io/badge/performance-38%25_faster-brightgreen)](https://kristjanvalur.github.io/fastcond/)

> **🚀 38% faster than pthread condition variables** — Because who doesn't want their threads to fly like the wind?

Fast, reliable POSIX condition variable alternatives built with semaphores. When `pthread_cond_t` just isn't cutting it, fastcond delivers the performance your applications deserve.

## 📈 Performance That Speaks for Itself

| Metric | pthread | **fastcond** | Improvement |
|--------|---------|-------------|-------------|
| **Throughput** | 403K items/s | **559K items/s** | **🔥 38% faster** |
| **Average Latency** | 11.9 μs | **8.8 μs** | **⚡ 26% lower** |
| **Tail Latency (max)** | 149 μs | **111 μs** | **📉 25% better** |

**🎯 [See Live Performance Dashboard →](https://kristjanvalur.github.io/fastcond/)**

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

## The Weak vs Strong Showdown 🥊

Two flavors of awesome, each with their own personality:

### 💪 **Strong Condition Variables** (`fastcond_cond_t`)
*The perfectionist of the family*
- **Strict POSIX semantics** — Only wakes threads that are already waiting (as they should!)
- **Lower latency** — Surprisingly, being picky makes it faster
- **Drop-in replacement** — Your pthread code will feel right at home
- **The people's choice** — Most applications want this one

### 🏃 **Weak Condition Variables** (`fastcond_wcond_t`) 
*The rebel with a cause*
- **Relaxed semantics** — Might wake a thread that just arrived to the party
- **Still faster than pthread** — Because even our "weak" is strong
- **Use when** — All your waiting threads are equivalent and interchangeable
- **For the adventurous** — When you know exactly what you're doing

> **Plot twist:** Both versions consistently outperform boring old `pthread_cond_t` on multicore systems. The "strong" version actually has *lower* latency despite doing more work. Go figure! 🤷‍♂️

## Why Choose Fastcond? 

Because life's too short for slow condition variables! 🐌→🚀

**The Numbers Don't Lie:**
- **38% higher throughput** — Your producer-consumer patterns will thank you
- **26% lower latency** — Messages zip through like they're caffeinated  
- **25% better tail latency** — Even your worst-case scenarios perform better
- **Consistent wins** — Not just cherry-picked benchmarks, real improvements across workloads

**Platform Compatibility:**
- ✅ **Linux** - Battle-tested and ready for production
- ⚠️ **macOS** - Mostly there (missing `*_timedwait` due to platform limitations)
- ❓ **Other POSIX** - Should work if you have proper semaphore support

**🎯 [Explore the Interactive Performance Dashboard →](https://kristjanvalur.github.io/fastcond/)**

*Featuring live benchmarks, beautiful charts, and all the performance porn your engineer heart desires.*

## Documentation

- **[docs/](docs/)** - User-facing documentation (performance results, API reference)
- **[dev-docs/](dev-docs/)** - Development documentation (benchmarking, CI/CD, deployment)
- **[BUILD.md](BUILD.md)** - Build instructions
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines
