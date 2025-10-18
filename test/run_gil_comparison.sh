#!/bin/bash

# GIL Comparative Test Script
# Runs both correctness and performance tests comparing:
# - fastcond vs native backends
# - fair vs unfair (plain mutex) behavior

echo "======================================================================"
echo "fastcond GIL Comprehensive Comparative Test Suite"
echo "======================================================================"
echo ""

# Default parameters
THREADS=${1:-4}
TOTAL_ACQUISITIONS=${2:-5000}
HOLD_TIME_US=${3:-100}
WORK_CYCLES=${4:-50}

echo "Configuration: $THREADS threads competing for $TOTAL_ACQUISITIONS acquisitions"
echo "Hold time: $HOLD_TIME_US μs, Work cycles: $WORK_CYCLES"
echo ""

# Run correctness tests
echo ">>> Running Correctness and Fairness Tests <<<"
echo ""

# Auto-detect if we're running from CMake build or make build
if [ -f "../build/gil_test_fc" ]; then
    # Running from project root, CMake build
    EXEC_PREFIX="../build/"
elif [ -f "./build/gil_test_fc" ]; then
    # Running from project root, CMake build (alt)
    EXEC_PREFIX="./build/"
elif [ -f "./gil_test_fc" ]; then
    # Running from test directory, make build
    EXEC_PREFIX="./"
else
    echo "Error: Could not find GIL test executables"
    exit 1
fi

echo "--- fastcond Backend (Fair) ---"
${EXEC_PREFIX}gil_test_fc $THREADS $TOTAL_ACQUISITIONS $HOLD_TIME_US $WORK_CYCLES

echo ""
echo "--- fastcond Backend (Unfair) ---"
${EXEC_PREFIX}gil_test_fc_unfair $THREADS $TOTAL_ACQUISITIONS $HOLD_TIME_US $WORK_CYCLES

echo ""
echo "--- Native pthread Backend (Fair) ---"
${EXEC_PREFIX}gil_test_native $THREADS $TOTAL_ACQUISITIONS $HOLD_TIME_US $WORK_CYCLES

echo ""
echo "--- Native pthread Backend (Unfair) ---"
${EXEC_PREFIX}gil_test_native_unfair $THREADS $TOTAL_ACQUISITIONS $HOLD_TIME_US $WORK_CYCLES

echo ""
echo ">>> Running Performance Benchmarks <<<"
echo ""

BENCH_ITERATIONS=1000

echo "--- fastcond Backend (Fair) Performance ---"
${EXEC_PREFIX}gil_benchmark_fc $THREADS $BENCH_ITERATIONS

echo ""
echo "--- fastcond Backend (Unfair) Performance ---"
${EXEC_PREFIX}gil_benchmark_fc_unfair $THREADS $BENCH_ITERATIONS

echo ""
echo "--- Native pthread Backend (Fair) Performance ---"
${EXEC_PREFIX}gil_benchmark_native $THREADS $BENCH_ITERATIONS

echo ""
echo "--- Native pthread Backend (Unfair) Performance ---"
${EXEC_PREFIX}gil_benchmark_native_unfair $THREADS $BENCH_ITERATIONS

echo ""
echo "======================================================================"
echo "Comprehensive Comparative Testing Complete"
echo "Test Matrix: 2 backends × 2 fairness modes × 2 test types = 8 tests"
echo "======================================================================"