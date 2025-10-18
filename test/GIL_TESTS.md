# GIL (Global Interpreter Lock) Tests

This directory contains comprehensive tests for the fastcond GIL implementation.

## Test Files

### Correctness and Fairness Tests

**`gil_test.c`** - Comprehensive correctness and fairness validation
- **Mutual exclusion**: Verifies only one thread holds GIL at any time
- **Fairness statistics**: Measures distribution of acquisitions across threads
- **Correctness under contention**: Tests behavior with multiple competing threads
- **Statistical fairness analysis**: Uses coefficient of variation and other metrics

**Usage:**
```bash
# fastcond backend
./gil_test_fc [num_threads] [total_acquisitions] [hold_time_us] [work_cycles]

# Native pthread backend  
./gil_test_native [num_threads] [total_acquisitions] [hold_time_us] [work_cycles]

# Example: 4 threads competing for 5000 acquisitions, 100μs hold time, 1000 work cycles
./gil_test_fc 4 5000 100 1000
```

**Test Design:**
- **Competitive acquisition**: All threads compete for a fixed total number of acquisitions
- **Realistic timing**: Includes sleep time while holding GIL to simulate I/O operations
- **Random delays**: Threads have randomized startup and inter-acquisition delays
- **True fairness measurement**: Distribution shows how fairly acquisitions are spread across threads

### Performance Benchmarks

**`gil_benchmark.c`** - Performance measurement under various contention scenarios
- **High contention**: Minimal hold times, maximum competition
- **Medium contention**: Moderate hold times, realistic workload
- **Low contention**: Longer hold times with gaps
- **Burst mode**: Rapid acquire/release cycles
- **Latency distribution**: Percentile analysis of acquire times

**Usage:**
```bash
# fastcond backend
./gil_benchmark_fc [num_threads] [iterations_per_thread]

# Native pthread backend
./gil_benchmark_native [num_threads] [iterations_per_thread]

# Example: 4 threads, 5000 iterations each
./gil_benchmark_fc 4 5000
```

### Comparative Testing

**`run_gil_comparison.sh`** - Automated comprehensive comparison script
- Runs both correctness and performance tests
- Compares fastcond vs native pthread backends
- Tests both fair and unfair (plain mutex) behavior
- Produces comprehensive 2×2×2 performance matrix

**Usage:**
```bash
./run_gil_comparison.sh [threads] [total_acquisitions] [hold_time_us] [work_cycles]

# Example: 
./run_gil_comparison.sh 4 5000 100 50
```

This script runs 8 total tests:
- 2 backends (fastcond vs native)
- 2 fairness modes (fair vs unfair) 
- 2 test types (correctness vs performance)

**Parameters:**
- `threads`: Number of competing threads (default: 4)
- `total_acquisitions`: Total GIL acquisitions to compete for (default: 5000)
- `hold_time_us`: Microseconds to sleep while holding GIL (default: 100)
- `work_cycles`: CPU work cycles while holding GIL (default: 50)

## Backend Selection

The GIL implementation supports two backends and two fairness modes via conditional compilation:

### Backend Control
- **fastcond backend** (default): Uses fastcond condition variables
- **Native backend**: Uses pthread condition variables

Backend selection is controlled by the `FASTCOND_GIL_USE_NATIVE_COND` macro:
- `0` (default): fastcond backend
- `1`: Native pthread backend

### Fairness Control
- **Fair mode** (default): Implements fairness mechanism to prevent greedy re-acquisition
- **Unfair mode**: Behaves like a plain mutex, allows greedy re-acquisition

Fairness control is controlled by the `FASTCOND_GIL_DISABLE_FAIRNESS` macro:
- `0` (default): Fairness enabled
- `1`: Fairness disabled (plain mutex behavior)

## Building Tests

Use the provided Makefile:

```bash
# Build all GIL tests (fair and unfair variants)
make gil_test_fc gil_test_native gil_test_fc_unfair gil_test_native_unfair 
make gil_benchmark_fc gil_benchmark_native gil_benchmark_fc_unfair gil_benchmark_native_unfair

# Or build specific tests
make gil_test_fc            # Correctness test with fastcond (fair)
make gil_test_fc_unfair     # Correctness test with fastcond (unfair)
make gil_test_native        # Correctness test with native pthread (fair)
make gil_test_native_unfair # Correctness test with native pthread (unfair)
make gil_benchmark_fc       # Benchmark with fastcond (fair)
make gil_benchmark_fc_unfair # Benchmark with fastcond (unfair)
make gil_benchmark_native   # Benchmark with native pthread (fair)
make gil_benchmark_native_unfair # Benchmark with native pthread (unfair)

# Clean up
make clean
```

## Test Interpretation

### Correctness Results

- **✅ Mutual exclusion: PASSED** - Only one thread held GIL at a time
- **❌ MUTUAL EXCLUSION VIOLATION** - Multiple threads held GIL (serious bug)
- **✅ Cleanup: PASSED** - No threads holding GIL after test completion

### Fairness Metrics

- **Fairness score**: Lower is better (0 = perfect fairness)
  - < 5.0: Excellent fairness
  - 5.0-15.0: Good fairness  
  - 15.0-30.0: Moderate fairness
  - > 30.0: Poor fairness (greedy behavior)

- **Consecutive re-acquisitions**: Percentage of times the same thread re-acquired GIL
- **Max consecutive by same thread**: Longest streak of same-thread acquisitions
- **Coefficient of variation**: Statistical measure of fairness (lower = more fair)

### Performance Metrics

- **Operations per second**: Total acquire/release cycles per second
- **Average latency**: Mean time to acquire GIL
- **Percentile latencies**: Distribution of acquire times (50th, 90th, 95th, 99th)
- **Max latency**: Worst-case acquire time

## Expected Results

### Fairness Comparison
- **Fair mode**: Should show excellent fairness due to the GIL's fairness algorithm that prevents greedy re-acquisition when other threads are waiting
- **Unfair mode**: May show poorer fairness, especially under high contention, as it behaves like a plain mutex allowing greedy re-acquisition

### Performance Comparison
- **fastcond**: May show lower latency due to optimized semaphore implementation
- **Native pthread**: Baseline performance using standard pthread condition variables
- **Fair vs Unfair**: Unfair mode may show higher throughput but poorer fairness
- **Platform differences**: Results may vary between Linux (POSIX) and macOS (GCD/pthread hybrid)

## Notes

- Tests use statistical analysis rather than absolute guarantees for fairness
- Scheduling and timing can affect results, especially with small thread counts
- Performance results are most meaningful with moderate to high contention
- The GIL fairness mechanism is probabilistic, not deterministic