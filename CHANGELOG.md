# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2025-10-26

### Changed
- **API Unification (Phase 1)**: `fastcond_wcond_t` is now an alias for `fastcond_cond_t`
  - Both variants now provide strong POSIX semantics (only wake already-waiting threads)
  - Weak variant removed as separate offering - performance data showed strong variant is actually faster
  - All `fastcond_wcond_*` functions are now wrappers calling `fastcond_cond_*` functions
  - Backward compatible: existing wcond API code continues to work with better semantics
- **Internal architecture**: Layered implementation with static inline `_weak_*` helper functions
  - Separate counter tracking: `w_waiting` (semaphore level), `n_waiting` (wait level), `n_wakeup` (pending wakeups)
  - Conceptually clear separation between weak primitive layer and strong bookkeeping layer

### Removed
- Weak variant documentation from README (no longer showcased as separate option)
- `*_wcond` test executable variants (now redundant - wcond is just an alias)
- Weak variant performance testing and CI test runs
- "Weak vs Strong Showdown" section explaining semantic differences

### Fixed
- `strongtest` with wcond patch now passes (previously would deadlock with true weak semantics)

## [0.2.1] - 2025-10-26

### Added
- **Cross-platform performance data collection and analysis infrastructure**
  - CSV-based data collection from CI builds across all platforms (Linux, macOS, Windows)
  - Automated benchmark artifact collection and merging from CI workflows
  - Python-based analysis tool (`analyze_performance.py`) generating comparison tables, charts, and JSON summaries
  - Automated GitHub Pages deployment with performance comparison visualizations
  - Standardized CSV schema for consistent cross-platform benchmark data

### Fixed
- CSV output format in `gil_test` to match standard schema (iterations column was missing)

## [0.2.0] - 2025-10-26
- **Windows platform support** with native CONDITION_VARIABLE and CRITICAL_SECTION implementations
- Native Windows semaphore implementation (CreateSemaphore/Wait/Release APIs)
- Cross-platform test portability layer (`test_portability.h`) for unified test code
- Platform abstraction via `native_primitives.h` for comparing fastcond against native implementations
- Windows CI integration with MSVC compiler in Debug and Release configurations
- Native baseline test variants (`_native` suffix) for all platforms
- Cross-platform `gil_benchmark` tool for GIL-specific performance testing
- **Windows API patching support** in `fastcond_patch.h` for CONDITION_VARIABLE macros
- **Windows-native millisecond wait functions** (`fastcond_*_wait_ms`) for zero-overhead timeout handling
- **Test instrumentation API** with callback mechanism to verify patch functionality (testing only)
- Comprehensive patch validation tests for both POSIX and Windows platforms

### Changed
- Renamed `_pt` (pthread) suffix to `_native` for platform-neutral baseline comparisons
- API redesigned to expose native mutex types per platform for optimal performance
- Improved GIL test synchronization and bookkeeping around `yield()` operations
- Applied consistent code formatting across all test files
- **Enhanced `fastcond_patch.h`** to support both Windows and POSIX APIs with platform detection
- Patch tests now compile fastcond.c directly with instrumentation for definitive verification

### Fixed
- ODR (One Definition Rule) violation in `gil_test_native` targets
- GIL bookkeeping bugs in `holder_count` tracking around yield operations
- Thread synchronization race conditions in `gil_test` startup
- Mutual exclusion violations on Windows (added volatile qualifiers)
- CSV output format in `gil_test` to match standard schema (iterations column was missing)

## [0.1.0] - 2025-01-15

### Added
- Initial release with Linux and macOS support
- Strong condition variables (`fastcond_cond_t`) with strict POSIX semantics
- Weak condition variables (`fastcond_wcond_t`) with relaxed semantics
- Global Interpreter Lock (GIL) implementation for Python-like threading patterns
- Comprehensive benchmark suite with JSON output and visualizations
- GitHub Actions CI with sanitizers (ASan, TSan, UBSan)
- GitHub Pages deployment for performance dashboards
- CMake build system with test integration

[0.2.1]: https://github.com/kristjanvalur/fastcond/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/kristjanvalur/fastcond/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/kristjanvalur/fastcond/releases/tag/v0.1.0
