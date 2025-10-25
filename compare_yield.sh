#!/bin/bash
# Simplified yield impact test - just builds both variants and runs them
# User can observe the output and compare manually

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="${SCRIPT_DIR}/test"

# Test parameters
DATA_COUNT=${1:-50000}
NUM_THREADS=${2:-4}
QUEUE_SIZE=${3:-10}

echo "======================================"
echo "FastCond YIELD() Impact Test"
echo "======================================"
echo ""
echo "Building two variants of fastcond:"
echo "  1. WITH yield (default)"
echo "  2. WITHOUT yield (FASTCOND_NO_YIELD=1)"
echo ""
echo "Test parameters: ${DATA_COUNT} items, ${NUM_THREADS} threads, queue size ${QUEUE_SIZE}"
echo ""

# Build variant WITH yield
echo "======================================"
echo "Building WITH YIELD variant..."
echo "======================================"
cd "${TEST_DIR}"
make clean > /dev/null 2>&1 || true
make CC="gcc -O2" PATCH=COND qtest_fc strongtest_fc
mv qtest_fc qtest_with_yield
mv strongtest_fc strongtest_with_yield
echo "✅ Built: qtest_with_yield, strongtest_with_yield"
echo ""

# Build variant WITHOUT yield
echo "======================================"
echo "Building WITHOUT YIELD variant..."
echo "======================================"
make clean > /dev/null 2>&1 || true
make CC="gcc -O2 -DFASTCOND_NO_YIELD=1" PATCH=COND qtest_fc strongtest_fc
mv qtest_fc qtest_no_yield
mv strongtest_fc strongtest_no_yield
echo "✅ Built: qtest_no_yield, strongtest_no_yield"
echo ""

# Run tests
echo "======================================"
echo "Running qtest WITH YIELD..."
echo "======================================"
./qtest_with_yield ${DATA_COUNT} ${NUM_THREADS} ${QUEUE_SIZE}
echo ""

echo "======================================"
echo "Running qtest WITHOUT YIELD..."
echo "======================================"
./qtest_no_yield ${DATA_COUNT} ${NUM_THREADS} ${QUEUE_SIZE}
echo ""

echo "======================================"
echo "Running strongtest WITH YIELD..."
echo "======================================"
./strongtest_with_yield 10000 5
echo ""

echo "======================================"
echo "Running strongtest WITHOUT YIELD..."
echo "======================================"
./strongtest_no_yield 10000 5
echo ""

echo "======================================"
echo "Tests complete!"
echo "======================================"
echo ""
echo "Compare the 'Throughput' values above to see the impact of YIELD()."
echo "Both strongtest variants should complete successfully (no deadlock)."
echo ""
echo "Test binaries are in: ${TEST_DIR}/"
echo "  - qtest_with_yield / qtest_no_yield"
echo "  - strongtest_with_yield / strongtest_no_yield"
echo ""
echo "To run more detailed analysis:"
echo "  ./qtest_with_yield 100000 8 10"
echo "  ./qtest_no_yield 100000 8 10"
