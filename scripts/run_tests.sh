#!/bin/bash
# Test runner script with clear pass/fail reporting
# Usage: ./run_tests.sh [quick|full]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test configuration
if [ "$1" = "quick" ]; then
    DATA_COUNT=100
    MODE="Quick"
elif [ "$1" = "full" ]; then
    DATA_COUNT=10000
    MODE="Full"
else
    DATA_COUNT=1000
    MODE="Standard"
fi

NUM_THREADS=4
QUEUE_SIZE=10

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}fastcond Test Suite (${MODE} Mode)${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Track results
PASSED=0
FAILED=0
TOTAL=0

# Function to run a test
run_test() {
    local name="$1"
    local cmd="$2"
    TOTAL=$((TOTAL + 1))
    
    echo -n "Running ${name}... "
    
    if output=$(eval "$cmd" 2>&1); then
        # Check if output contains expected patterns
        if echo "$output" | grep -q "sender.*sent" && echo "$output" | grep -q "receiver.*got"; then
            echo -e "${GREEN}✓ PASSED${NC}"
            PASSED=$((PASSED + 1))
            return 0
        else
            echo -e "${RED}✗ FAILED${NC} (unexpected output)"
            echo "$output" | sed 's/^/  /'
            FAILED=$((FAILED + 1))
            return 1
        fi
    else
        echo -e "${RED}✗ FAILED${NC} (exit code $?)"
        echo "$output" | sed 's/^/  /'
        FAILED=$((FAILED + 1))
        return 1
    fi
}

# Determine test directory
if [ -d "build" ]; then
    TEST_DIR="build"
elif [ -d "test" ]; then
    TEST_DIR="test"
else
    echo -e "${RED}Error: Cannot find test executables${NC}"
    echo "Run 'cmake --build build' or 'make -C test' first"
    exit 1
fi

# Change to test directory
cd "$TEST_DIR" 2>/dev/null || true

# Find test executables
QTEST_PT=$(find . -name "qtest_pt" -type f 2>/dev/null | head -1)
QTEST_FC=$(find . -name "qtest_fc" -type f 2>/dev/null | head -1)
QTEST_WCOND=$(find . -name "qtest_wcond" -type f 2>/dev/null | head -1)
STRONGTEST_PT=$(find . -name "strongtest_pt" -type f 2>/dev/null | head -1)
STRONGTEST_FC=$(find . -name "strongtest_fc" -type f 2>/dev/null | head -1)
STRONGTEST_WCOND=$(find . -name "strongtest_wcond" -type f 2>/dev/null | head -1)

if [ -z "$QTEST_PT" ] || [ -z "$QTEST_FC" ]; then
    echo -e "${RED}Error: Test executables not found${NC}"
    echo "Build the tests first with 'cmake --build build' or 'make -C test'"
    exit 1
fi

echo -e "${YELLOW}Test Configuration:${NC}"
echo "  Data count: $DATA_COUNT"
echo "  Threads: $NUM_THREADS"
echo "  Queue size: $QUEUE_SIZE"
echo ""

# Run qtest variants
echo -e "${BLUE}--- Producer-Consumer Queue Tests ---${NC}"
run_test "qtest (pthread)" "$QTEST_PT $DATA_COUNT $NUM_THREADS $QUEUE_SIZE"
run_test "qtest (fastcond strong)" "$QTEST_FC $DATA_COUNT $NUM_THREADS $QUEUE_SIZE"
if [ -n "$QTEST_WCOND" ]; then
    run_test "qtest (fastcond weak)" "$QTEST_WCOND $DATA_COUNT $NUM_THREADS $QUEUE_SIZE"
fi
echo ""

# Run strongtest variants
echo -e "${BLUE}--- Strong Semantics Tests ---${NC}"
run_test "strongtest (pthread)" "$STRONGTEST_PT $DATA_COUNT $QUEUE_SIZE"
run_test "strongtest (fastcond strong)" "$STRONGTEST_FC $DATA_COUNT $QUEUE_SIZE"
# NOTE: strongtest with weak variant is skipped because it will deadlock.
# strongtest specifically tests that only waiting threads get woken (no stealing),
# which the weak variant cannot guarantee. This is expected behavior.
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Test Summary${NC}"
echo -e "${BLUE}========================================${NC}"
echo "Total tests: $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED${NC}"
else
    echo "Failed: 0"
fi
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ✓${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed! ✗${NC}"
    exit 1
fi
