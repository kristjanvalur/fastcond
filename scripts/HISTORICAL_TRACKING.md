# Historical Benchmark Tracking

This directory contains tools for tracking fastcond performance trends over time.

## Overview

Performance benchmarks can vary between runs due to system load, hardware variance, and measurement noise. To determine if fastcond provides consistent performance benefits, we track all benchmark results in a time-series database stored in a separate `benchmark-data` git branch.

## Tools

### `store_benchmark_history.py`

Stores benchmark results in historical database.

**Usage:**
```bash
# Store main benchmark results
python store_benchmark_history.py ../docs/benchmark-results.json \
  --history-type benchmark \
  --run-id "20250127-180000"

# Store cross-platform results
python store_benchmark_history.py ../performance-output/performance-summary.json \
  --history-type platform \
  --run-id "20250127-180000"
```

**What it does:**
- Appends results to `benchmark-history.json` or `platform-history.json` in `benchmark-data` branch
- Includes timestamp, git commit SHA, and full benchmark metadata
- Automatically creates `benchmark-data` branch if it doesn't exist
- Commits and pushes changes (use `--no-push` for local testing)

### `analyze_benchmark_trends.py`

Analyzes historical performance trends with statistical rigor.

**Usage:**
```bash
# Analyze trends and generate report + charts
python analyze_benchmark_trends.py benchmark-history.json \
  --output-dir trend-analysis

# Just generate report, no charts
python analyze_benchmark_trends.py benchmark-history.json \
  --output-dir trend-analysis \
  --no-charts
```

**What it produces:**
- **`trend-analysis.md`**: Markdown report with:
  - Mean speedup and standard deviation across all runs
  - Min/max speedup values
  - Trend direction (improving/declining/stable)
  - Statistical significance testing (t-test)
  
- **`{benchmark}_{platform}-trend.png`**: Charts showing:
  - Speedup over time with trend line
  - Mean speedup with ±1 std dev confidence band
  - Baseline (1.0×) reference line

## CI/CD Integration

Both workflows automatically store results:

1. **Main benchmarks** (`benchmarks.yml`):
   - Runs on schedule (weekly) or manual trigger
   - Stores to `benchmark-history.json`

2. **Cross-platform** (`ci.yml`):
   - Runs on every push to master
   - Stores to `platform-history.json`

## Accessing Historical Data

```bash
# Clone the benchmark-data branch
git fetch origin benchmark-data
git checkout benchmark-data

# View historical data
cat benchmark-history.json
cat platform-history.json
```

## Statistical Interpretation

The analysis provides:

- **Mean Speedup**: Average performance ratio (fastcond / native)
- **Standard Deviation**: Measurement variance/stability
- **Statistical Significance**: Whether speedup is real or measurement noise
  - t-statistic > 2.0 indicates p < 0.05 (95% confidence)
  - Shows if performance difference is statistically significant

**Example interpretation:**
```
Mean Speedup: 1.082× (+8.2%)
Std Deviation: 0.034×
t-statistic: 3.45
Result: ✅ Statistically significant
```

This means fastcond is consistently ~8% faster with high confidence, not just measurement noise.

## Local Testing

```bash
# Test historical storage without pushing
cd scripts
uv run python store_benchmark_history.py \
  ../docs/benchmark-results.json \
  --history-type benchmark \
  --no-push

# Analyze existing historical data
git checkout benchmark-data
uv run python analyze_benchmark_trends.py \
  ../benchmark-history.json \
  --output-dir ../trend-analysis
```

## Why This Matters

Current benchmark pages show small performance differences (~1-3%) which could be within measurement variance (CV% of 3-9%). Historical tracking answers:

1. **Is the speedup real or noise?** - Statistical significance testing
2. **Is performance improving over time?** - Trend analysis with slope
3. **How stable are the results?** - Standard deviation across runs
4. **Are there platform-specific differences?** - Separate tracking per OS

This transforms uncertain single-run results into statistically confident trends.
