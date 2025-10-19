#!/bin/bash
set -e

# GIL Fairness Analysis Publication Script
# Generates comprehensive fairness analysis and prepares for GitHub Pages publication

echo "🎭 FastCond GIL Fairness Analysis Publication"
echo "=============================================="

# Set up directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DOCS_DIR="$PROJECT_DIR/docs"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"

mkdir -p "$DOCS_DIR"

echo "📁 Project directory: $PROJECT_DIR"
echo "📊 Publication directory: $DOCS_DIR"
echo "🔨 Build directory: $BUILD_DIR"
echo

# Check if GIL tests are built
echo "🔍 Checking GIL test availability..."
required_tests=("gil_test_fc" "gil_test_fc_unfair" "gil_test_fc_naive")
missing_tests=()

for test in "${required_tests[@]}"; do
    if [[ ! -f "$BUILD_DIR/$test" ]]; then
        missing_tests+=("$test")
    fi
done

if [[ ${#missing_tests[@]} -gt 0 ]]; then
    echo "❌ Missing GIL tests: ${missing_tests[*]}"
    echo "   Building required tests..."
    cd "$PROJECT_DIR"
    cmake --build "$BUILD_DIR" --target "${missing_tests[@]}"
    echo "✅ GIL tests built successfully"
fi

# Run GIL fairness benchmarks
echo "🚀 Running GIL fairness benchmarks..."
cd "$SCRIPT_DIR"

# Ensure uv environment is ready
if [[ ! -d ".venv" ]]; then
    echo "📦 Setting up Python environment..."
    uv sync
fi

# Generate comprehensive benchmark data including all three modes
echo "   📊 Generating fairness benchmark data..."
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
GIL_RESULTS_FILE="$DOCS_DIR/gil-fairness-results.json"

# Use the updated benchmark script that includes all three modes
uv run benchmark_json.py "$BUILD_DIR" | python3 -c "
import json
import sys

# Read the full benchmark data (already an array)
data = json.load(sys.stdin)

# Filter for only GIL tests
gil_data = [result for result in data if result.get('benchmark') == 'gil_test']

# Add generation metadata
metadata = {
    'generated_at': '$TIMESTAMP',
    'generator': 'run_gil_fairness_analysis.sh',
    'total_tests': len(gil_data)
}

# Output in the same format as benchmark_json.py expects
print(json.dumps(gil_data, indent=2))
" > "$GIL_RESULTS_FILE"

# Validate that we got all three modes
echo "   🔍 Validating benchmark results..."
MODES_FOUND=$(python3 -c "
import json
with open('$GIL_RESULTS_FILE', 'r') as f:
    data = json.load(f)
modes = set()
for result in data:
    mode = result.get('fairness_mode')
    if mode:
        modes.add(mode)
print(','.join(sorted(modes)))
")

echo "   📋 Found fairness modes: $MODES_FOUND"

if [[ "$MODES_FOUND" != "fair,naive,unfair" ]]; then
    echo "❌ Error: Expected 'fair,naive,unfair' but got '$MODES_FOUND'"
    echo "   Check that all GIL test variants are properly built and configured"
    exit 1
fi

echo "   ✅ All three fairness modes captured successfully"

# Generate HTML page
echo "   📄 Generating HTML fairness analysis page..."
uv run generate_gil_fairness_html.py "$GIL_RESULTS_FILE" --output "$DOCS_DIR/gil-fairness.html"

# Generate the fairness chart if visualization script exists
if [[ -f "visualize_gil_fairness.py" ]]; then
    echo "   📈 Generating fairness visualization chart..."
    uv run visualize_gil_fairness.py "$GIL_RESULTS_FILE" --output-dir "$DOCS_DIR"
    
    # Move the chart to expected location
    if [[ -f "$DOCS_DIR/gil_fairness_comparison.png" ]]; then
        echo "   ✅ Fairness chart generated successfully"
    else
        echo "   ⚠️  Warning: Fairness chart generation may have failed"
    fi
else
    echo "   ⚠️  Visualization script not found, skipping chart generation"
fi

# Generate summary information
echo "   📋 Generating summary statistics..."
SUMMARY=$(python3 -c "
import json
with open('$GIL_RESULTS_FILE', 'r') as f:
    data = json.load(f)

modes = {}

for result in data:
    mode = result.get('fairness_mode')
    if mode:
        fairness_stats = result.get('results', {}).get('fairness_stats', {})
        overall_stats = result.get('results', {}).get('overall', {})
        modes[mode] = {
            'cv': fairness_stats.get('coefficient_of_variation', 0),
            'ops_per_sec': overall_stats.get('operations_per_sec', 0),
            'transitions_pct': fairness_stats.get('transition_percentage', 0)
        }

# Calculate key insights
if 'fair' in modes and 'unfair' in modes:
    perf_diff = ((modes['unfair']['ops_per_sec'] / modes['fair']['ops_per_sec']) - 1) * 100
    fairness_improvement = modes['unfair']['cv'] / modes['fair']['cv'] if modes['fair']['cv'] > 0 else float('inf')
    
    print(f\"Fair CV: {modes['fair']['cv']:.3f}\")
    print(f\"Unfair CV: {modes['unfair']['cv']:.3f}\")
    print(f\"Naive CV: {modes.get('naive', {}).get('cv', 'N/A')}\")
    print(f\"Performance cost of fairness: {perf_diff:.1f}%\")
    print(f\"Fairness improvement ratio: {fairness_improvement:.1f}x\")
")

echo "$SUMMARY"

echo
echo "🎉 GIL Fairness Analysis Publication Complete!"
echo "=============================================="
echo "📊 Results: $GIL_RESULTS_FILE"
echo "📄 HTML Page: $DOCS_DIR/gil-fairness.html"
if [[ -f "$DOCS_DIR/gil_fairness_comparison.png" ]]; then
    echo "📈 Chart: $DOCS_DIR/gil_fairness_comparison.png"
fi
echo
echo "🌐 To publish to GitHub Pages:"
echo "   - The files in $DOCS_DIR/ will be automatically deployed"
echo "   - Access via: https://kristjanvalur.github.io/fastcond/gil-fairness.html"
echo
echo "💡 Key Insights:"
echo "$SUMMARY" | sed 's/^/   /'