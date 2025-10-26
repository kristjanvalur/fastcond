#!/bin/bash
# Performance Benchmark Runner
# 
# This script runs performance benchmarks and optionally outputs CSV data.
# Can be used both locally and in CI environments.
#
# Usage:
#   ./run_performance_benchmarks.sh [BUILD_DIR] [OUTPUT_CSV]
#
# Environment variables:
#   FASTCOND_PLATFORM - Platform name (default: auto-detect)
#   FASTCOND_OS_VERSION - OS version (default: auto-detect)
#   BUILD_DIR - Directory containing test executables (default: build or test)
#   OUTPUT_CSV - Output CSV file path (default: performance-results.csv)

set -e

# Determine build directory
if [ -n "$1" ]; then
    BUILD_DIR="$1"
elif [ -n "$BUILD_DIR" ]; then
    : # Use BUILD_DIR from environment
elif [ -d "build" ]; then
    BUILD_DIR="build"
elif [ -d "test" ]; then
    BUILD_DIR="test"
else
    echo "Error: Cannot find build directory. Please specify BUILD_DIR."
    exit 1
fi

# Determine output CSV file
OUTPUT_CSV="${2:-${OUTPUT_CSV:-performance-results.csv}}"

# Auto-detect platform if not set
if [ -z "$FASTCOND_PLATFORM" ]; then
    case "$(uname -s)" in
        Linux*)     FASTCOND_PLATFORM="linux";;
        Darwin*)    FASTCOND_PLATFORM="macos";;
        CYGWIN*)    FASTCOND_PLATFORM="windows";;
        MINGW*)     FASTCOND_PLATFORM="windows";;
        MSYS*)      FASTCOND_PLATFORM="windows";;
        *)          FASTCOND_PLATFORM="unknown";;
    esac
fi

# Auto-detect OS version if not set
if [ -z "$FASTCOND_OS_VERSION" ]; then
    if [ "$FASTCOND_PLATFORM" = "linux" ]; then
        FASTCOND_OS_VERSION="$(lsb_release -ds 2>/dev/null || cat /etc/os-release 2>/dev/null | grep PRETTY_NAME | cut -d'"' -f2 || echo 'unknown')"
    elif [ "$FASTCOND_PLATFORM" = "macos" ]; then
        FASTCOND_OS_VERSION="$(sw_vers -productVersion 2>/dev/null || echo 'unknown')"
    else
        FASTCOND_OS_VERSION="unknown"
    fi
fi

# Export for test programs
export FASTCOND_PLATFORM
export FASTCOND_OS_VERSION
export FASTCOND_CSV_OUTPUT="$OUTPUT_CSV"

# Determine executable extension
if [ "$FASTCOND_PLATFORM" = "windows" ]; then
    EXT=".exe"
else
    EXT=""
fi

echo "=========================================="
echo "fastcond Performance Benchmark Suite"
echo "=========================================="
echo "Platform:     $FASTCOND_PLATFORM"
echo "OS Version:   $FASTCOND_OS_VERSION"
echo "Build Dir:    $BUILD_DIR"
echo "Output CSV:   $OUTPUT_CSV"
echo "=========================================="
echo ""

# Initialize CSV file with header
echo "platform,os_version,test,variant,threads,param,iterations,elapsed_sec,throughput" > "$OUTPUT_CSV"

# Function to run a benchmark safely
run_benchmark() {
    local executable="$1"
    shift
    local args="$@"
    
    if [ -x "$executable" ]; then
        echo "Running: $(basename $executable) $args"
        "$executable" $args || {
            echo "  ⚠️  Benchmark failed (exit code $?), continuing..."
            return 0
        }
        echo ""
    else
        echo "  ⚠️  Executable not found or not executable: $executable"
        echo ""
    fi
}

# Run qtest benchmarks
echo "=== Producer-Consumer Tests (qtest) ==="
echo "Testing with 10,000 items, 4 threads, queue size 10"
echo ""

run_benchmark "$BUILD_DIR/qtest_native${EXT}" 10000 4 10
run_benchmark "$BUILD_DIR/qtest_fc${EXT}" 10000 4 10
run_benchmark "$BUILD_DIR/qtest_wcond${EXT}" 10000 4 10

# Run strongtest benchmarks
echo "=== Single Condition Tests (strongtest) ==="
echo "Testing with 10,000 items, queue size 5"
echo ""

run_benchmark "$BUILD_DIR/strongtest_native${EXT}" 10000 5
run_benchmark "$BUILD_DIR/strongtest_fc${EXT}" 10000 5

# Run gil_test benchmarks
echo "=== GIL Tests (gil_test) ==="
echo "Testing with 4 threads, 1,000 acquisitions"
echo ""

run_benchmark "$BUILD_DIR/gil_test_fc${EXT}" 4 1000
run_benchmark "$BUILD_DIR/gil_test_native${EXT}" 4 1000
run_benchmark "$BUILD_DIR/gil_test_native_unfair${EXT}" 4 1000

echo "=========================================="
echo "Benchmark run complete!"
echo "=========================================="
echo ""

# Show collected data
if [ -f "$OUTPUT_CSV" ]; then
    line_count=$(wc -l < "$OUTPUT_CSV")
    data_lines=$((line_count - 1))
    
    echo "Collected $data_lines benchmark results:"
    echo ""
    cat "$OUTPUT_CSV"
    echo ""
    
    if [ $data_lines -eq 0 ]; then
        echo "⚠️  Warning: No benchmark data was collected!"
        echo "This may indicate that test executables were not found or failed to run."
        exit 1
    else
        echo "✅ Performance data saved to: $OUTPUT_CSV"
    fi
else
    echo "❌ Error: Output CSV file was not created: $OUTPUT_CSV"
    exit 1
fi
