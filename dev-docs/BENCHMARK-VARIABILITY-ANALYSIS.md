# Benchmark Variability Analysis

**Date**: October 27, 2025  
**System**: Ubuntu Linux x86_64, 16 cores

## Executive Summary

Current benchmarks show **significant run-to-run variability** (5-9% coefficient of variation) due to very short test duration (~25ms). This makes single-run comparisons unreliable and explains why local results differ from CI results.

## Measured Variability

### QTEST (10K items, 4 threads, queue size 10)

**Native pthread:**
- Mean: 397,334 items/sec
- Standard Deviation: 21,839 items/sec
- **Coefficient of Variation: 5.5%**
- Range: 372,282 - 430,426 items/sec (14.6% variation)
- Runtime: ~25ms

**Fastcond:**
- Mean: 370,022 items/sec  
- Standard Deviation: 31,739 items/sec
- **Coefficient of Variation: 8.6%**
- Range: 338,551 - 416,637 items/sec (21.1% variation)
- Runtime: ~25-30ms

**Local Result**: Native **6.9% faster** than fastcond (opposite to CI showing fastcond +55.6% faster!)

### STRONGTEST (10K items, queue size 5)

**Native pthread:**
- Mean: 1,128,244 items/sec
- **Coefficient of Variation: 5.2%**
- Range: 1,044,065 - 1,189,723 items/sec (12.9% variation)

**Fastcond:**
- Mean: 1,095,016 items/sec
- **Coefficient of Variation: 3.9%**
- Range: 1,035,388 - 1,132,918 items/sec (8.9% variation)

**Local Result**: Native **2.9% faster** than fastcond

## Root Causes

### 1. Very Short Test Duration
- Tests complete in 25-30 milliseconds
- Too brief to average out:
  - CPU frequency scaling
  - Cache effects
  - Context switches
  - System load fluctuations
  - NUMA effects

### 2. Single-Run Measurement
- `benchmark_json.py` runs each test **exactly once**
- No warm-up runs to prime caches
- No averaging across multiple iterations
- No outlier detection/removal

### 3. Sequential Execution
- Tests run as separate processes (`qtest_native`, `qtest_fc`)
- Different thermal/frequency states between runs
- System state changes between variants

### 4. No Statistical Analysis
- No confidence intervals reported
- No significance testing
- Single-point measurements treated as absolute truth

## Recommendations

### Immediate Improvements

#### 1. Increase Test Data Size
```bash
# Current: 10,000 items (~25ms)
./qtest_native 10000 4 10

# Proposed: 1,000,000 items (~2.5s)
./qtest_native 1000000 4 10
```

**Rationale**: Longer tests (1-10 seconds) average out system noise. Aim for at least 1 second runtime.

#### 2. Multiple Iterations with Statistics
Modify `benchmark_json.py` to run each test N times:

```python
def run_benchmark_iterations(executable, args, iterations=5, warmup=1):
    """Run benchmark multiple times and return statistics."""
    results = []
    
    # Warm-up runs (discard)
    for _ in range(warmup):
        subprocess.run([executable] + args, capture_output=True)
    
    # Measurement runs
    for _ in range(iterations):
        result = subprocess.run([executable] + args, capture_output=True, text=True)
        throughput = extract_throughput(result.stdout)
        results.append(throughput)
    
    return {
        "mean": statistics.mean(results),
        "stdev": statistics.stdev(results),
        "min": min(results),
        "max": max(results),
        "cv_percent": (statistics.stdev(results) / statistics.mean(results)) * 100,
        "individual_runs": results
    }
```

**Configuration**:
- Warmup runs: 1-2
- Measurement runs: 5-10
- Report: mean, stdev, confidence interval

#### 3. Interleaved Execution
Run native and fastcond alternately to reduce bias from thermal/system state changes:

```
Run 1: native, fc
Run 2: native, fc  
Run 3: native, fc
...
```

### Medium-Term Improvements

#### 4. Historical Data Storage
Track benchmark results over time:

```json
{
  "commit": "286440f",
  "timestamp": "2025-10-27T11:30:00Z",
  "system": {...},
  "results": {
    "qtest_native": {
      "mean": 397334,
      "stdev": 21839,
      "runs": [...]
    },
    "qtest_fc": {...}
  }
}
```

Store in:
- `docs/benchmark-history/` directory
- Time-series JSON files
- Generate trend charts showing performance over time

#### 5. Regression Detection
Compare current results against historical baseline:

```python
def detect_regression(current_mean, historical_mean, historical_stdev):
    """Detect if performance has regressed significantly."""
    z_score = (current_mean - historical_mean) / historical_stdev
    
    if z_score < -2:  # 95% confidence
        return "REGRESSION"
    elif z_score > 2:
        return "IMPROVEMENT"
    return "STABLE"
```

#### 6. Confidence Intervals in Reports
Replace single-point measurements with ranges:

```
Average Performance Gain: +55.6% ± 8.3%
(95% confidence interval: +47.3% to +63.9%)
```

### Long-Term Improvements

#### 7. Automated Performance CI Gates
Fail CI if performance regresses beyond threshold:

```yaml
- name: Check for performance regression
  run: |
    python scripts/check_regression.py \
      --baseline docs/benchmark-history/baseline.json \
      --current docs/benchmark-results.json \
      --threshold 10  # Fail if >10% regression
```

#### 8. Cross-System Normalization
Normalize results to reduce cross-system variation:
- Report speedup ratios (fastcond/native) instead of absolute throughput
- Calibrate using known-stable benchmark workloads

#### 9. Statistical Power Analysis
Determine optimal number of iterations needed for desired confidence level based on measured CV.

## Implementation Priority

### Phase 1 (High Impact, Low Effort)
1. ✅ Increase test data size to 100K-1M items
2. ✅ Add multiple iterations (5 runs) with mean/stdev reporting

### Phase 2 (Medium Effort)
3. ⏳ Store historical data in `docs/benchmark-history/`
4. ⏳ Generate trend charts
5. ⏳ Report confidence intervals on GitHub Pages

### Phase 3 (Long-term)
6. ⏳ Automated regression detection in CI
7. ⏳ Statistical significance testing
8. ⏳ Cross-platform result normalization

## Example Output Format

```
QTEST Results (10 iterations, 1M items):
  Native:     mean=423,442 ± 12,334 items/sec  (CV=2.9%)
  Fastcond:   mean=658,921 ± 18,772 items/sec  (CV=2.8%)
  
  Speedup: +55.6% ± 4.2%  (95% CI: [51.4%, 59.8%])
  Statistical significance: p < 0.001 (highly significant)
```

## Questions for Discussion

1. **Target test duration**: 1 second? 5 seconds? 10 seconds?
2. **Number of iterations**: 5? 10? 20?
3. **Historical data retention**: How many commits back?
4. **Regression threshold**: 5%? 10%? 15%?
5. **CI failure policy**: Block merge on regression?

## References

- [Rigorous Benchmarking in Reasonable Time](https://www.cse.unsw.edu.au/~gernot/benchmarking-crimes.html)
- [How to Benchmark Code in C](https://easyperf.net/blog/2018/04/03/Precise-timing-of-machine-code-with-Linux-perf)
- Statistical analysis: Bessel's correction, confidence intervals, t-tests
