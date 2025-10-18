# Building fastcond

This document describes how to build and test the fastcond library.

## Quick Start

### Using CMake (Recommended)

```bash
# Configure and build
cmake -B build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Run benchmarks
cmake --build build --target benchmark
```

### Using Make (Traditional)

```bash
cd test
make all
./run_tests.sh
```

## Build Options

### CMake Options

- `BUILD_SHARED_LIBS` - Build shared library instead of static (default: OFF)
- `FASTCOND_BUILD_TESTS` - Build test executables (default: ON)
- `FASTCOND_BUILD_BENCHMARKS` - Build benchmark targets (default: ON)
- `CMAKE_BUILD_TYPE` - Build type: Release, Debug, RelWithDebInfo (default: Release)

Example:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON
```

### Makefile Options

- `PATCH=COND` - Use strong condition variable (default)
- `PATCH=WCOND` - Use weak condition variable
- `CFLAGS` - Compiler flags (default: `-O3`)

Example:
```bash
cd test
make PATCH=WCOND CFLAGS="-O2 -g"
```

## Build Configurations

### Debug Build (with symbols)

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

### Release Build (optimized)

```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

### With Address Sanitizer

```bash
cmake -B build-asan \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
cmake --build build-asan
cd build-asan && ctest
```

### With Thread Sanitizer

```bash
cmake -B build-tsan \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fsanitize=thread"
cmake --build build-tsan
cd build-tsan && ctest
```

## Testing

### Quick Smoke Tests

```bash
# Using CMake/CTest
ctest --test-dir build

# Using test script
./scripts/run_tests.sh quick
```

### Full Test Suite

```bash
./scripts/run_tests.sh full
```

### Individual Tests

```bash
# Using CMake-built binaries
./build/qtest_fc 10000 4 10
./build/strongtest_fc 10000 5

# Using Make-built binaries
cd test
./qtest_fc 10000 4 10
./strongtest_fc 10000 5
```

Test arguments:
- **qtest**: `<data_count> <num_threads> <queue_size>`
- **strongtest**: `<data_count> <queue_size>`

**Note on strongtest with weak variant**: The `strongtest_wcond` test is intentionally 
not run in the test suite because it will deadlock. This is expected behavior - strongtest 
specifically validates that only already-waiting threads are woken up (no wakeup stealing), 
which the weak condition variable cannot guarantee. The weak variant should only be used 
when all waiting threads are functionally equivalent.

## Benchmarking

### Using CMake

```bash
cmake --build build --target benchmark
```

### Using script

```bash
./scripts/benchmark.sh
```

### Manual benchmarking

```bash
cd build  # or test/

echo "=== Pthread baseline ==="
./qtest_pt 10000 4 10

echo "=== Fastcond (strong) ==="
./qtest_fc 10000 4 10

echo "=== Fastcond (weak) ==="
./qtest_wcond 10000 4 10
```

## Installation

### System-wide installation

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build
```

### Custom installation location

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build
cmake --install build
```

This installs:
- Headers: `$PREFIX/include/fastcond/`
- Library: `$PREFIX/lib/libfastcond.a`
- CMake config: `$PREFIX/lib/cmake/fastcond/`

## Using fastcond in Your Project

### With CMake

```cmake
find_package(fastcond REQUIRED)
target_link_libraries(your_target PRIVATE fastcond::fastcond)
```

### With pkg-config

```bash
gcc myapp.c $(pkg-config --cflags --libs fastcond) -o myapp
```

### Manual linking

```bash
gcc -I/usr/local/include/fastcond myapp.c -L/usr/local/lib -lfastcond -lpthread -o myapp
```

## Cross-Platform Notes

### Linux
Standard build should work out of the box. This is the primary supported platform.

### macOS
**Fully supported.** Uses GCD dispatch semaphores (`dispatch_semaphore_t`) internally as a workaround for deprecated POSIX unnamed semaphores (`sem_init`, `sem_timedwait`). All other primitives (mutexes, condition variables, thread IDs) use standard pthread APIs.

The performance characteristics on macOS may differ from Linux due to the GCD semaphore implementation, making cross-platform benchmarking particularly interesting.

### Windows (WSL/MinGW)
May work with appropriate POSIX compatibility layer. Not regularly tested.

## Code Formatting

The project uses `clang-format` for consistent code style. CI will check formatting.

### Format all code

```bash
./scripts/format.sh
```

Or manually:
```bash
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -i
```

### Check formatting without modifying files

```bash
find fastcond test -name "*.c" -o -name "*.h" | xargs clang-format -n -Werror
```

This will report errors if files need formatting (same check as CI).

### Install clang-format

```bash
# Ubuntu/Debian
sudo apt install clang-format

# macOS
brew install clang-format
```

## Troubleshooting

### "pthread not found"
Install pthread development package:
```bash
# Ubuntu/Debian
sudo apt-get install libpthread-stubs0-dev

# Usually included in build-essential
sudo apt-get install build-essential
```

### Test executables not found
Build them first:
```bash
cmake --build build
# or
make -C test all
```

### CMake version too old
Requires CMake 3.10+. Update with:
```bash
# Ubuntu 20.04+
sudo apt-get install cmake

# Or install latest from cmake.org
```

## Clean Build

```bash
# CMake
rm -rf build/

# Make
make -C test clean
```
