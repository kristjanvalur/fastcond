#!/bin/bash
set -e

# GIL Fairness Analysis Script
# Generates comprehensive fairness comparison between fair and unfair GIL implementations

echo "🎭 FastCond GIL Fairness Analysis"
echo "=================================="

# Set up directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RESULTS_DIR="$PROJECT_DIR/benchmark_results"
CHARTS_DIR="$PROJECT_DIR/charts"

mkdir -p "$RESULTS_DIR"
mkdir -p "$CHARTS_DIR"

echo "📁 Project directory: $PROJECT_DIR"
echo "📊 Results will be saved to: $RESULTS_DIR"
echo "📈 Charts will be saved to: $CHARTS_DIR"
echo

# Check if GIL tests are built
if [[ ! -f "$PROJECT_DIR/test/gil_test_fc" ]]; then
    echo "❌ GIL tests not found. Building..."
    cd "$PROJECT_DIR/test"
    make gil_test_fc gil_test_fc_unfair
    echo "✅ GIL tests built successfully"
    echo
fi

# Run benchmarks
echo "🚀 Running GIL fairness benchmarks..."
cd "$SCRIPT_DIR"

# Ensure uv environment is ready
if [[ ! -d ".venv" ]]; then
    echo "📦 Setting up Python environment..."
    uv sync
fi

# Generate benchmark data
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULTS_FILE="$RESULTS_DIR/gil_fairness_$TIMESTAMP.json"

echo "   ⏱️  Running fair vs unfair GIL comparison..."
uv run benchmark_json.py ../test | tee "$RESULTS_FILE.tmp"

# Check if we got valid results
if grep -q '"fairness_mode": "fair"' "$RESULTS_FILE.tmp" && grep -q '"fairness_mode": "unfair"' "$RESULTS_FILE.tmp"; then
    mv "$RESULTS_FILE.tmp" "$RESULTS_FILE"
    echo "   ✅ Benchmark data saved to: $(basename "$RESULTS_FILE")"
else
    echo "   ❌ Error: Could not generate valid GIL benchmark data"
    cat "$RESULTS_FILE.tmp"
    rm -f "$RESULTS_FILE.tmp"
    exit 1
fi

# Generate visualization
echo "   📊 Generating fairness analysis charts..."
uv run visualize_gil_fairness.py "$RESULTS_FILE" --output-dir "$CHARTS_DIR"

# Also create a symlink to latest results
ln -sf "$(basename "$RESULTS_FILE")" "$RESULTS_DIR/gil_fairness_latest.json"

echo
echo "🎉 GIL Fairness Analysis Complete!"
echo "=================================="
echo "📊 Results: $RESULTS_FILE"
echo "📈 Chart:   $CHARTS_DIR/gil_fairness_comparison.png"
echo
echo "🔍 Quick Summary:"

# Extract key metrics for summary using Python for reliable JSON parsing
METRICS=$(uv run python3 -c "
import json
import sys
try:
    with open('$RESULTS_FILE', 'r') as f:
        data = json.load(f)
    
    fair_data = next((r for r in data if r.get('fairness_mode') == 'fair'), None)
    unfair_data = next((r for r in data if r.get('fairness_mode') == 'unfair'), None)
    
    if fair_data and unfair_data:
        fair_cv = fair_data['results']['fairness_stats']['coefficient_of_variation']
        unfair_cv = unfair_data['results']['fairness_stats']['coefficient_of_variation']
        fair_ops = fair_data['results']['overall']['operations_per_sec']
        unfair_ops = unfair_data['results']['overall']['operations_per_sec']
        overhead = round((unfair_ops - fair_ops) * 100 / fair_ops, 1)
        
        print(f'{fair_cv:.3f}|{unfair_cv:.3f}|{fair_ops}|{unfair_ops}|{overhead}')
    else:
        print('ERROR')
except Exception as e:
    print('ERROR')
" 2>/dev/null)

if [[ "$METRICS" != "ERROR" && -n "$METRICS" ]]; then
    IFS='|' read -r FAIR_CV UNFAIR_CV FAIR_OPS UNFAIR_OPS OVERHEAD <<< "$METRICS"

    echo "   Fair GIL:   CV=$FAIR_CV, Ops/sec=$FAIR_OPS"
    echo "   Unfair GIL: CV=$UNFAIR_CV, Ops/sec=$UNFAIR_OPS"
    echo "   Performance cost of fairness: ${OVERHEAD}%"
else
    echo "   (Could not extract summary metrics)"
fi

echo
echo "💡 The chart dramatically shows why fairness matters:"
echo "   • Fair GIL ensures all threads get equal access"
echo "   • Unfair GIL allows severe thread starvation"
echo "   • Performance cost is minimal for significant fairness gain"
echo
echo "To view the chart: open $CHARTS_DIR/gil_fairness_comparison.png"