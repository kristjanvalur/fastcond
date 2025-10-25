#!/bin/bash
# Test script to compare fastcond performance with and without YIELD()
# in the strong condition variable spurious wakeup path

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="${SCRIPT_DIR}/test"
BUILD_DIR="${SCRIPT_DIR}/build-yield-test"

# Test parameters
DATA_COUNT=50000
NUM_THREADS=4
QUEUE_SIZE=10
NUM_RUNS=5

echo "======================================"
echo "FastCond YIELD() Impact Analysis"
echo "======================================"
echo ""
echo "Test Configuration:"
echo "  Data count: ${DATA_COUNT}"
echo "  Threads: ${NUM_THREADS}"
echo "  Queue size: ${QUEUE_SIZE}"
echo "  Runs per variant: ${NUM_RUNS}"
echo ""

# Function to build and test a variant
run_variant() {
    local variant_name=$1
    local cflags=$2
    local desc=$3
    
    echo "======================================"
    echo "Testing: ${variant_name}"
    echo "Description: ${desc}"
    echo "CFLAGS: ${cflags}"
    echo "======================================"
    
    # Clean and rebuild
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    # Build library with specific flags
    echo "Building fastcond library..."
    gcc -c ${cflags} -O2 -Wall -Wextra -I"${SCRIPT_DIR}/fastcond" \
        "${SCRIPT_DIR}/fastcond/fastcond.c" -o fastcond.o
    ar rcs libfastcond.a fastcond.o
    
    # Build test executables
    echo "Building test programs..."
    gcc ${cflags} -O2 -Wall -Wextra -DFASTCOND_PATCH_COND \
        -I"${SCRIPT_DIR}/fastcond" "${TEST_DIR}/qtest.c" \
        -L. -lfastcond -lpthread -lm -o qtest_fc
    
    gcc ${cflags} -O2 -Wall -Wextra -DFASTCOND_PATCH_COND \
        -I"${SCRIPT_DIR}/fastcond" "${TEST_DIR}/strongtest.c" \
        -L. -lfastcond -lpthread -lm -o strongtest_fc
    
    # Run tests multiple times and collect results
    echo ""
    echo "Running qtest (producer-consumer latency)..."
    local total_latency=0
    local total_throughput=0
    
    for run in $(seq 1 ${NUM_RUNS}); do
        echo -n "  Run ${run}/${NUM_RUNS}: "
        output=$(./qtest_fc ${DATA_COUNT} ${NUM_THREADS} ${QUEUE_SIZE} 2>&1)
        
        # Extract latency (in microseconds)
        latency=$(echo "$output" | grep "Average latency" | awk '{print $3}')
        # Extract throughput (items/sec)
        throughput=$(echo "$output" | grep "throughput" | awk '{print $2}')
        
        echo "latency=${latency}μs, throughput=${throughput} items/sec"
        
        # Accumulate for average (using bc for floating point)
        total_latency=$(echo "$total_latency + $latency" | bc)
        total_throughput=$(echo "$total_throughput + $throughput" | bc)
    done
    
    # Calculate averages
    avg_latency=$(echo "scale=3; $total_latency / $NUM_RUNS" | bc)
    avg_throughput=$(echo "scale=0; $total_throughput / $NUM_RUNS" | bc)
    
    echo ""
    echo "Results for ${variant_name}:"
    echo "  Average latency: ${avg_latency} μs"
    echo "  Average throughput: ${avg_throughput} items/sec"
    echo ""
    
    # Run strongtest once for verification (should not deadlock)
    echo "Running strongtest (single condition, fairness)..."
    timeout 10s ./strongtest_fc 10000 5 > /dev/null 2>&1 && \
        echo "  ✅ strongtest completed successfully" || \
        echo "  ❌ strongtest failed or timed out"
    
    echo ""
    
    # Return to script directory
    cd "${SCRIPT_DIR}"
    
    # Export results for comparison
    eval "${variant_name}_latency=${avg_latency}"
    eval "${variant_name}_throughput=${avg_throughput}"
}

# Test variant 1: WITH yield (default behavior)
run_variant "with_yield" "" "Default: Uses YIELD() in spurious wakeup path"

# Test variant 2: WITHOUT yield
run_variant "no_yield" "-DFASTCOND_NO_YIELD=1" "Optimized: No YIELD() in spurious wakeup path"

# Comparison summary
echo "======================================"
echo "COMPARISON SUMMARY"
echo "======================================"
echo ""
echo "Latency (lower is better):"
echo "  With YIELD:    ${with_yield_latency} μs"
echo "  Without YIELD: ${no_yield_latency} μs"

# Calculate percentage difference
latency_diff=$(echo "scale=2; (($no_yield_latency - $with_yield_latency) / $with_yield_latency) * 100" | bc)
if (( $(echo "$latency_diff < 0" | bc -l) )); then
    echo "  Improvement: ${latency_diff#-}% faster without YIELD ✨"
elif (( $(echo "$latency_diff > 0" | bc -l) )); then
    echo "  Regression: ${latency_diff}% slower without YIELD ⚠️"
else
    echo "  No significant difference"
fi

echo ""
echo "Throughput (higher is better):"
echo "  With YIELD:    ${with_yield_throughput} items/sec"
echo "  Without YIELD: ${no_yield_throughput} items/sec"

throughput_diff=$(echo "scale=2; (($no_yield_throughput - $with_yield_throughput) / $with_yield_throughput) * 100" | bc)
if (( $(echo "$throughput_diff > 0" | bc -l) )); then
    echo "  Improvement: ${throughput_diff}% higher without YIELD ✨"
elif (( $(echo "$throughput_diff < 0" | bc -l) )); then
    echo "  Regression: ${throughput_diff#-}% lower without YIELD ⚠️"
else
    echo "  No significant difference"
fi

echo ""
echo "======================================"
echo "RECOMMENDATION"
echo "======================================"

# Determine recommendation based on latency improvement
if (( $(echo "$latency_diff < -2" | bc -l) )); then
    echo "💡 Removing YIELD() shows measurable improvement (>${latency_diff#-}%)"
    echo "   Consider removing it if strongtest consistently passes."
elif (( $(echo "$latency_diff > 2" | bc -l) )); then
    echo "⚠️  Removing YIELD() shows degradation (${latency_diff}%)"
    echo "   Keep YIELD() for better fairness under contention."
else
    echo "📊 Performance difference is negligible (<2%)"
    echo "   Keep YIELD() for safety unless every microsecond matters."
fi

echo ""
echo "Test completed. Build artifacts in: ${BUILD_DIR}"
