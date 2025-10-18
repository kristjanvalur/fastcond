# Using the Benchmarking System

## 🎯 Quick Reference

### Local Usage
```bash
# One-time setup
cd scripts && uv sync && cd ..

# Run benchmarks
./scripts/run_benchmarks.sh

# Results are in docs/
open docs/benchmark-results.md
open docs/throughput-comparison.png
```

### CI/CD Usage

#### Setup (One Time)
```bash
# Enable GitHub Pages
./scripts/setup_github_pages.sh

# Then in GitHub: Settings → Pages → Source: gh-pages
```

#### Automatic Triggers
- ✅ **Push to master**: Full benchmarks + GitHub Pages deployment
- ✅ **Pull requests**: Performance regression detection
- ✅ **Manual**: GitHub Actions UI → "Performance Benchmarks" → "Run workflow"
- ✅ **Weekly**: Every Monday (tracks performance over time)

#### View Results

**Option 1: GitHub Pages (Public)**
```
https://<username>.github.io/fastcond/
```

**Option 2: CI Artifacts**
1. Go to Actions tab
2. Click workflow run
3. Download `benchmark-results-<sha>.zip`

**Option 3: PR Comments**
- Automatic comments with performance comparison
- 🟢 >2% improvement | ⚪ <2% change | 🔴 >2% regression

## 📁 Files

```
.github/workflows/benchmarks.yml  # CI workflow
scripts/
  ├── benchmark_json.py           # Parse output to JSON
  ├── visualize.py                # Create charts
  ├── run_benchmarks.sh          # Run full suite
  ├── setup_github_pages.sh      # One-time setup
  ├── pyproject.toml             # Python dependencies
  ├── uv.lock                    # Locked dependencies
  └── .venv/                     # Virtual environment
dev-docs/
  ├── benchmarking.md            # Local usage guide
  └── ci-benchmarks.md           # CI/CD complete guide
```

## 📊 What You Get

### JSON Data
```json
{
  "benchmark": "qtest",
  "implementation": "fastcond_strong",
  "results": {
    "overall": {
      "throughput_items_per_sec": 577906.14
    }
  }
}
```

### Markdown Tables
| Implementation | Throughput | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 577,906 items/sec | +31.9% |
| pthread | 438,260 items/sec | baseline |

### PNG Charts
- `throughput-comparison.png` - Bar chart
- `latency-comparison.png` - Bar chart with error bars
- `gil_fairness_comparison.png` - Fair vs unfair GIL analysis

## 🎭 GIL Fairness Analysis

### Quick GIL Test
```bash
# Fair GIL (recommended)
cd test && ./gil_test_fc

# Unfair GIL (demonstrates thread starvation)
cd test && ./gil_test_fc_unfair
```

### Generate Fairness Charts
```bash
cd scripts
uv run benchmark_json.py ../test > ../benchmark_results/gil_latest.json
uv run visualize_gil_fairness.py ../benchmark_results/gil_latest.json --output-dir ../charts
```

### What You'll See
- **Fair GIL**: Perfect thread distribution (coefficient of variation ≈ 0.0)
- **Unfair GIL**: Severe thread starvation (some threads get 0 acquisitions!)
- **Performance**: Fair GIL adds ~3-6% overhead for dramatically improved fairness

The fairness visualization highlights why the anti-greedy mechanism matters in real applications.

## 🔗 Documentation

- **[benchmarking.md](benchmarking.md)** - Complete local benchmarking guide
- **[ci-benchmarks.md](ci-benchmarks.md)** - Complete CI/CD integration guide
- **[performance-analysis.md](docs/performance-analysis.md)** - Performance analysis and insights

## 🚀 Quick Examples

### Run specific test manually
```bash
# Build first
cd test && make qtest_fc

# Run with custom params
./qtest_fc 50000 8 5  # 50K items, 8 threads, queue size 5
```

### Generate charts from existing JSON
```bash
cd scripts
uv run visualize.py my-results.json --output-dir ../charts/
```

### Compare two benchmark runs
```bash
# Run baseline
git checkout main
./scripts/run_benchmarks.sh
mv docs/benchmark-results.json /tmp/baseline.json

# Run changes
git checkout feature-branch
./scripts/run_benchmarks.sh

# Compare (in Python)
cd scripts
uv run python -c "
import json
base = json.load(open('/tmp/baseline.json'))
new = json.load(open('../docs/benchmark-results.json'))
# ... comparison logic
"
```

## ❓ Need Help?

- **Local issues**: See [benchmarking.md](benchmarking.md)
- **CI issues**: See [ci-benchmarks.md](ci-benchmarks.md)
- **Build issues**: See [BUILD.md](BUILD.md)
