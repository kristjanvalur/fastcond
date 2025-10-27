#!/bin/bash
#
# Run complete performance benchmark suite and generate visualizations
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"
OUTPUT_DIR="${OUTPUT_DIR:-$PROJECT_DIR/docs}"

# Convert BUILD_DIR to absolute path if it's relative
if [[ "$BUILD_DIR" != /* ]]; then
    BUILD_DIR="$PROJECT_DIR/$BUILD_DIR"
fi

echo "==================================="
echo "fastcond Performance Benchmark Suite"
echo "==================================="
echo ""
echo "Build directory: $BUILD_DIR"
echo "Output directory: $OUTPUT_DIR"
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found: $BUILD_DIR"
    echo "Please build the project first or set BUILD_DIR environment variable"
    exit 1
fi

# Check if executables exist
REQUIRED_EXES=(
    "$BUILD_DIR/qtest_native"
    "$BUILD_DIR/qtest_fc"
    "$BUILD_DIR/strongtest_native"
    "$BUILD_DIR/strongtest_fc"
)

missing=0
for exe in "${REQUIRED_EXES[@]}"; do
    if [ ! -x "$exe" ]; then
        echo "Error: Required executable not found: $exe"
        missing=1
    fi
done

if [ $missing -eq 1 ]; then
    echo ""
    echo "Please build all test targets first:"
    echo "  cd test && make all"
    exit 1
fi

echo "Running benchmarks..."
echo ""

# Generate JSON results using robust benchmark runner
# Uses 400K items (~1s runtime), 5 iterations with 1 warm-up run
JSON_FILE="$OUTPUT_DIR/benchmark-results.json"
(cd "$SCRIPT_DIR" && uv run benchmark_runner.py "$BUILD_DIR" --iterations 5 --warmup 1 --items 400000) > "$JSON_FILE"

if [ $? -ne 0 ]; then
    echo "Error: Benchmark execution failed"
    exit 1
fi

echo "✓ Benchmark data saved to: $JSON_FILE"
echo ""

# Generate visualizations
echo "Generating visualizations..."
(cd "$SCRIPT_DIR" && uv run visualize.py "$JSON_FILE" --output-dir "$OUTPUT_DIR")

if [ $? -ne 0 ]; then
    echo "Error: Visualization generation failed"
    exit 1
fi

# Generate HTML performance page
echo "Generating HTML performance page..."
echo "📁 Output directory: $OUTPUT_DIR"
echo "📊 Input JSON file: $JSON_FILE"
echo "🌐 Target HTML file: $OUTPUT_DIR/index.html"
echo "📂 Current working directory: $(pwd)"
echo "📋 Files in output directory before HTML generation:"
ls -la "$OUTPUT_DIR" 2>/dev/null || echo "Output directory not accessible"
(cd "$SCRIPT_DIR" && uv run generate_html.py "$JSON_FILE" --output "$OUTPUT_DIR/index.html")

if [ $? -ne 0 ]; then
    echo "❌ Error: HTML page generation failed"
    exit 1
fi

echo "✅ HTML generation completed"
echo "📋 Files in output directory after HTML generation:"
ls -la "$OUTPUT_DIR"

echo ""
echo "==================================="
echo "Benchmark suite completed!"
echo "==================================="
echo ""
echo "Results:"
echo "  - JSON data: $JSON_FILE"
echo "  - Markdown table: $OUTPUT_DIR/benchmark-results.md"
echo "  - HTML performance page: $OUTPUT_DIR/index.html"
echo "  - Throughput chart: $OUTPUT_DIR/throughput-comparison.png"
echo "  - Latency chart: $OUTPUT_DIR/latency-comparison.png"
echo ""
