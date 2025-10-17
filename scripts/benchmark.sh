#!/bin/bash
# Benchmark runner with comparison output
# Usage: ./benchmark.sh

set -e

# Colors
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}fastcond Performance Benchmark${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Determine test directory
if [ -d "build" ]; then
    TEST_DIR="build"
elif [ -d "test" ]; then
    TEST_DIR="test"
else
    echo "Error: Cannot find test executables"
    exit 1
fi

cd "$TEST_DIR" 2>/dev/null || true

# Find executables
QTEST_PT=$(find . -name "qtest_pt" -type f 2>/dev/null | head -1)
QTEST_FC=$(find . -name "qtest_fc" -type f 2>/dev/null | head -1)
STRONGTEST_PT=$(find . -name "strongtest_pt" -type f 2>/dev/null | head -1)
STRONGTEST_FC=$(find . -name "strongtest_fc" -type f 2>/dev/null | head -1)

if [ -z "$QTEST_PT" ] || [ -z "$QTEST_FC" ]; then
    echo "Error: Test executables not found. Build them first."
    exit 1
fi

# Benchmark configuration
DATA_COUNT=10000
NUM_THREADS=4
QUEUE_SIZE=10

echo -e "${YELLOW}Configuration:${NC}"
echo "  Data count: $DATA_COUNT"
echo "  Threads: $NUM_THREADS"
echo "  Queue size: $QUEUE_SIZE"
echo ""

# qtest benchmark
echo -e "${BLUE}--- Producer-Consumer Latency Test ---${NC}"
echo ""
echo "Pthread implementation:"
$QTEST_PT $DATA_COUNT $NUM_THREADS $QUEUE_SIZE
echo ""
echo "Fastcond (strong) implementation:"
$QTEST_FC $DATA_COUNT $NUM_THREADS $QUEUE_SIZE
echo ""

# strongtest benchmark
echo -e "${BLUE}--- Single Condition Variable Test ---${NC}"
echo ""
echo "Pthread implementation:"
$STRONGTEST_PT $DATA_COUNT $QUEUE_SIZE
echo ""
echo "Fastcond (strong) implementation:"
$STRONGTEST_FC $DATA_COUNT $QUEUE_SIZE
echo ""

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Benchmark Complete${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Compare the latency statistics above."
echo "Lower average and standard deviation indicate better performance."
