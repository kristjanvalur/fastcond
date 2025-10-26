# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-10-26

### Added
- **Windows platform support** with native CONDITION_VARIABLE and CRITICAL_SECTION implementations
- Native Windows semaphore implementation (CreateSemaphore/Wait/Release APIs)
- Cross-platform test portability layer (`test_portability.h`) for unified test code
- Platform abstraction via `native_primitives.h` for comparing fastcond against native implementations
- Windows CI integration with MSVC compiler in Debug and Release configurations
- Native baseline test variants (`_native` suffix) for all platforms
- Cross-platform `gil_benchmark` tool for GIL-specific performance testing
- **Windows API patching support** in `fastcond_patch.h` for CONDITION_VARIABLE macros

### Changed
- Renamed `_pt` (pthread) suffix to `_native` for platform-neutral baseline comparisons
- API redesigned to expose native mutex types per platform for optimal performance
- Improved GIL test synchronization and bookkeeping around `yield()` operations
- Applied consistent code formatting across all test files
- **Enhanced `fastcond_patch.h`** to support both Windows and POSIX APIs with platform detection

### Fixed
- ODR (One Definition Rule) violation in `gil_test_native` targets
- GIL bookkeeping bugs in `holder_count` tracking around yield operations
- Thread synchronization race conditions in `gil_test` startup
- Mutual exclusion violations on Windows (added volatile qualifiers)

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

[0.2.0]: https://github.com/kristjanvalur/fastcond/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/kristjanvalur/fastcond/releases/tag/v0.1.0
