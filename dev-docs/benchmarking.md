# Performance Benchmarking

This directory contains tools for performance testing and visualization of the fastcond library.

## Overview

The benchmarking system consists of:

1. **C test programs** (`qtest`, `strongtest`) - Generate performance data
2. **Python scripts** - Parse output, generate JSON, create visualizations
3. **Automated runner** - Execute complete benchmark suite

## Quick Start

### Prerequisites

- Built fastcond test executables (see [BUILD.md](../BUILD.md))
- [uv](https://github.com/astral-sh/uv) package manager

### Install Dependencies

```bash
cd /path/to/fastcond/scripts
uv sync
```

This installs matplotlib and other dependencies in a local virtual environment within the scripts directory.

### Run Benchmarks

```bash
./scripts/run_benchmarks.sh
```

This will:
1. Execute all test variants (pthread, fastcond_strong, fastcond_weak)
2. Generate JSON results in `docs/benchmark-results.json`
3. Create comparison tables in `docs/benchmark-results.md`
4. Generate charts in `docs/throughput-comparison.png` and `docs/latency-comparison.png`

## Manual Usage

### Generate JSON from Test Output

```bash
cd scripts
uv run benchmark_json.py ../build > results.json
```

Arguments:
- `../build` - Build directory containing test executables (relative to scripts/)

### Visualize Results

```bash
cd scripts
uv run visualize.py results.json --output-dir ../docs
```

Options:
- `--output-dir` - Where to save charts and tables (default: `docs`)
- `--no-charts` - Skip chart generation, only create markdown table

## Test Programs

### qtest - Producer-Consumer Queue Test

Tests multi-threaded producer-consumer scenario with separate `not_empty` and `not_full` conditions.

**Parameters:**
```bash
./build/qtest_<variant> <data_count> <num_threads> <queue_size>
```

Example:
```bash
./build/qtest_fc 10000 4 10
```
- 10,000 items total
- 4 sender threads + 4 receiver threads
- Queue capacity of 10

**Measures:**
- Per-receiver latency (avg, stdev, min, max)
- Overall throughput (items/sec)
- Total execution time

### strongtest - Strong Semantics Test

Tests single condition variable shared by both producer and consumer, validating that only already-waiting threads are awakened (strong semantics).

**Parameters:**
```bash
./build/strongtest_<variant> <data_count> <queue_size>
```

Example:
```bash
./build/strongtest_fc 10000 5
```
- 10,000 items total
- 1 sender thread + 1 receiver thread
- Queue capacity of 5

**Measures:**
- Receiver latency (avg, stdev, min, max)
- Overall throughput (items/sec)
- Total execution time

**Important:** Do NOT run `strongtest_wcond` - it will deadlock because the weak variant cannot guarantee strong semantics.

## Test Variants

Each test has three implementation variants:

| Suffix | Implementation | Description |
|--------|---------------|-------------|
| `_pt` | pthread | Standard POSIX condition variables (baseline) |
| `_fc` | fastcond_strong | fastcond strong variant with full POSIX semantics |
| `_wcond` | fastcond_weak | fastcond weak variant (may wake not-yet-waiting threads) |

## Output Format

### JSON Structure

```json
{
  "benchmark": "qtest",
  "implementation": "pthread",
  "timestamp": "2025-10-17T18:16:00.947713",
  "system": {
    "os": "Linux",
    "arch": "x86_64",
    "python_version": "3.12.3",
    "cores": 16
  },
  "config": {
    "total_items": 10000,
    "num_senders": 4,
    "num_receivers": 4,
    "queue_size": 10
  },
  "results": {
    "per_thread": [
      {
        "thread_id": 0,
        "thread_type": "receiver",
        "items": 2510,
        "avg_latency_sec": 9.909952e-06,
        "stdev_latency_sec": 1.407137e-05,
        "min_latency_sec": 4.7e-08,
        "max_latency_sec": 0.000135517
      }
    ],
    "overall": {
      "total_time_sec": 0.023284,
      "throughput_items_per_sec": 429476.34
    }
  }
}
```

### Markdown Tables

The visualize script generates comparison tables showing:

- **Throughput Comparison:** Items/sec with speedup percentage vs pthread
- **Latency Comparison:** Average, min, max, and standard deviation in microseconds

### Charts

Two PNG charts are generated:

1. **throughput-comparison.png** - Horizontal bar chart of throughput (K items/sec)
2. **latency-comparison.png** - Horizontal bar chart of average latency (μs) with error bars

## Interpreting Results

### Expected Performance Characteristics

**Multi-threaded (qtest with 4+ threads):**
- `fastcond_strong` should show 20-40% better throughput than `pthread`
- `fastcond_strong` should show 20-30% lower latency than `pthread`
- `fastcond_weak` may be slower in mixed producer-consumer scenarios due to wakeup stealing

**Single-threaded (strongtest):**
- Performance should be similar between all implementations
- Slight variations are normal and not significant

### Performance Factors

- **Number of threads:** More contention = bigger fastcond advantage
- **Queue size:** Smaller queues = more synchronization = bigger advantage
- **CPU cores:** More cores = better parallel performance
- **System load:** Run on idle system for consistent results

## Extending the Benchmarks

### Adding New Tests

1. Create test program in `test/` directory
2. Add build targets to `test/Makefile`
3. Update `scripts/benchmark_json.py` to parse new output format
4. Add test configuration to `benchmarks` list in `benchmark_json.py`

### Custom Configurations

Edit `scripts/benchmark_json.py` and modify the `benchmarks` list:

```python
benchmarks = [
    {
        'name': 'qtest',
        'variants': ['pt', 'fc', 'wcond'],
        'args': ['50000', '8', '5'],  # More items, threads, smaller queue
        'description': 'Heavy load test'
    }
]
```

## Continuous Integration

The project includes automated CI/CD benchmarking with:
- Automatic runs on commits and PRs
- Performance regression detection
- GitHub Pages deployment for public results
- Artifact archival

**See [ci-benchmarks.md](ci-benchmarks.md) for complete CI/CD setup and usage.**

Quick start:
```bash
./scripts/setup_github_pages.sh
```

## Dependencies

Managed by `uv` via `pyproject.toml`:

- **matplotlib** - Chart generation
- **numpy** - Numerical computations (matplotlib dependency)

To add more dependencies:
```bash
cd scripts
uv add <package-name>
```

## Troubleshooting

**Charts not generated:**
- Check that matplotlib is installed: `uv sync`
- Verify uv is using the correct Python: `uv run python --version`

**Test executables not found:**
- Build tests first: `cd test && make all`
- Check BUILD_DIR environment variable

**Permission denied on run_benchmarks.sh:**
- Make executable: `chmod +x scripts/run_benchmarks.sh`

**Inconsistent results:**
- Run on idle system
- Close unnecessary applications
- Run multiple times and average results
- Consider increasing data_count for more stable measurements
