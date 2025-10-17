# CI/CD Benchmarking Integration

This document explains how to use the automated benchmarking system in CI/CD pipelines and publish results online.

## Overview

The fastcond project includes automated performance benchmarking that:

1. **Runs on every commit** to master/main branches
2. **Runs on pull requests** with performance regression detection
3. **Publishes results** to GitHub Pages for public viewing
4. **Archives results** as workflow artifacts
5. **Posts comparisons** as PR comments

## Quick Start

### 1. Enable GitHub Pages

```bash
# Run the setup script
./scripts/setup_github_pages.sh
```

Then enable GitHub Pages in your repository:
1. Go to **Settings** → **Pages**
2. Set **Source** to `gh-pages` branch
3. Click **Save**

Your benchmarks will be published to:
```
https://<username>.github.io/fastcond/
```

### 2. Trigger Benchmarks

Benchmarks run automatically on:
- **Push to master/main**: Full benchmark suite with GitHub Pages deployment
- **Pull requests**: Benchmark comparison with base branch
- **Manual trigger**: Via GitHub Actions UI
- **Weekly schedule**: Every Monday at 00:00 UTC (tracks performance over time)

## CI Workflows

### Performance Benchmarks Workflow

**File:** `.github/workflows/benchmarks.yml`

#### Jobs:

##### 1. `benchmark` - Run Full Benchmark Suite
- Builds tests in Release mode with optimizations
- Runs all test variants (pthread, fastcond_strong, fastcond_weak)
- Generates JSON, markdown, and PNG charts
- Uploads results as artifacts
- Posts results to PR (if applicable)
- Deploys to GitHub Pages (master only)

**Artifacts:**
- `benchmark-results-<commit-sha>`
  - `benchmark-results.json` - Structured data
  - `benchmark-results.md` - Comparison tables
  - `throughput-comparison.png` - Chart
  - `latency-comparison.png` - Chart

**Retention:** 90 days

##### 2. `compare-baseline` - Performance Regression Detection
- Runs only on pull requests
- Benchmarks both PR and base branch
- Compares throughput changes
- Posts comparison comment with emojis:
  - 🟢 Green: >2% improvement
  - 🔴 Red: >2% regression
  - ⚪ White: <2% change (normal variance)

**Example PR Comment:**
```markdown
## 🔬 Performance Regression Check

## Performance Comparison vs Base Branch

🟢 **qtest_fastcond_strong**: +5.2% (580,123 vs 551,234 items/sec)
⚪ **qtest_pthread**: +0.8% (431,234 vs 427,890 items/sec)
🔴 **strongtest_fastcond_strong**: -3.1% (1,135,678 vs 1,171,234 items/sec)

---
*Changes > ±2% are highlighted*
```

## GitHub Pages Structure

After deployment, your GitHub Pages site will have:

```
https://<username>.github.io/fastcond/
├── index.html                           # Landing page
├── benchmarks/
│   ├── <commit-sha-1>/
│   │   ├── benchmark-results.json
│   │   ├── benchmark-results.md
│   │   ├── throughput-comparison.png
│   │   └── latency-comparison.png
│   ├── <commit-sha-2>/
│   │   └── ...
│   └── ...
```

The index page automatically redirects to the latest benchmark results.

## Manual Benchmark Runs

You can also run benchmarks locally and upload them:

```bash
# Run benchmarks
./scripts/run_benchmarks.sh

# Results are in docs/
ls docs/benchmark-results.*
ls docs/*-comparison.png
```

## Viewing Results

### Option 1: GitHub Actions Artifacts

1. Go to **Actions** tab in GitHub
2. Click on a workflow run
3. Scroll to **Artifacts** section
4. Download `benchmark-results-<commit-sha>.zip`

### Option 2: GitHub Pages (Public)

Visit your GitHub Pages URL:
```
https://<username>.github.io/fastcond/
```

Direct link to specific commit:
```
https://<username>.github.io/fastcond/benchmarks/<commit-sha>/benchmark-results.md
```

### Option 3: Pull Request Comments

Performance results are automatically posted as comments on PRs.

### Option 4: Raw JSON API

Access raw JSON data:
```bash
# Via GitHub Pages
curl https://<username>.github.io/fastcond/benchmarks/<commit-sha>/benchmark-results.json

# Via GitHub API (artifacts)
gh api repos/<username>/fastcond/actions/artifacts
```

## Integrating with Other Services

### Continuous Benchmarking Platforms

You can integrate with services like:

#### 1. **Bencher.dev**

Add to `.github/workflows/benchmarks.yml`:

```yaml
- name: Track continuous benchmarks
  uses: bencherdev/bencher@main
  with:
    bencher-api-token: ${{ secrets.BENCHER_API_TOKEN }}
    project: fastcond
    testbed: ubuntu-latest
    adapter: json
    file: docs/benchmark-results.json
```

#### 2. **CodeSpeed**

```yaml
- name: Upload to CodeSpeed
  uses: CodSpeedHQ/action@v2
  with:
    token: ${{ secrets.CODSPEED_TOKEN }}
    results-file: docs/benchmark-results.json
```

#### 3. **Performance Dashboard**

Create a custom dashboard using the JSON data:

```python
import requests
import json

# Fetch latest results
url = "https://<username>.github.io/fastcond/benchmarks/latest/benchmark-results.json"
data = requests.get(url).json()

# Process and display
for result in data:
    print(f"{result['benchmark']} ({result['implementation']}): "
          f"{result['results']['overall']['throughput_items_per_sec']:,.0f} items/sec")
```

## Customizing Benchmarks

### Change Test Parameters

Edit `scripts/benchmark_json.py`:

```python
benchmarks = [
    {
        'name': 'qtest',
        'variants': ['pt', 'fc', 'wcond'],
        'args': ['50000', '8', '5'],  # More items, threads, smaller queue
        'description': 'Heavy load test (50K items, 8 threads, queue size 5)'
    }
]
```

### Add Custom Metrics

Extend `parse_qtest_output()` in `scripts/benchmark_json.py`:

```python
def parse_qtest_output(output):
    results = {
        'per_thread': [],
        'overall': {},
        'custom_metrics': {}  # Add your metrics here
    }
    # ... parsing logic
```

### Modify Charts

Edit `scripts/visualize.py` to customize chart appearance:

```python
# Change colors
color_map = {
    'pthread': '#e74c3c',        # Red
    'fastcond_strong': '#27ae60',  # Green
    'fastcond_weak': '#f39c12'     # Orange
}

# Adjust chart size
fig, axes = plt.subplots(1, len(by_benchmark), figsize=(8*len(by_benchmark), 6))
```

## Performance Tracking Over Time

### Historical Analysis

Collect results from multiple runs:

```bash
# Download artifacts from multiple CI runs
for sha in commit1 commit2 commit3; do
    gh run download --name benchmark-results-$sha
done

# Combine results
python scripts/historical_analysis.py *.json > trend.md
```

### Time Series Charts

Create a script to plot performance over time:

```python
import json
import matplotlib.pyplot as plt
from datetime import datetime

results = []
for file in sorted(glob.glob('benchmark-results-*.json')):
    with open(file) as f:
        data = json.load(f)
        results.append(data)

# Plot throughput over time
timestamps = [datetime.fromisoformat(r['timestamp']) for r in results]
throughputs = [r['results']['overall']['throughput_items_per_sec'] for r in results]

plt.plot(timestamps, throughputs)
plt.title('Performance Over Time')
plt.ylabel('Throughput (items/sec)')
plt.show()
```

## Troubleshooting

### Benchmarks Not Running

**Check workflow file syntax:**
```bash
# Validate YAML
yamllint .github/workflows/benchmarks.yml
```

**Check uv installation:**
```yaml
- name: Debug uv
  run: |
    which uv
    uv --version
```

### GitHub Pages Not Updating

**Verify deployment:**
1. Check Actions tab for deployment job status
2. Ensure `GITHUB_TOKEN` has Pages write permissions
3. Verify `gh-pages` branch exists

**Manual deployment:**
```bash
git checkout gh-pages
git merge master --strategy-option theirs
git push origin gh-pages
```

### Inconsistent Results

**Run multiple iterations:**
```yaml
- name: Run benchmarks (3 iterations)
  run: |
    for i in 1 2 3; do
      echo "=== Iteration $i ==="
      ./scripts/run_benchmarks.sh
      mv docs/benchmark-results.json docs/results-$i.json
    done
    # Average results
    uv run python scripts/average_results.py docs/results-*.json > docs/benchmark-results.json
```

**Use CPU pinning (Linux):**
```bash
taskset -c 0-3 ./scripts/run_benchmarks.sh
```

## Best Practices

1. **Always run in Release mode** with optimizations for accurate benchmarks
2. **Run multiple iterations** and average results to reduce variance
3. **Use consistent hardware** (same runner type) for comparisons
4. **Set performance thresholds** (e.g., ±2%) to identify real regressions
5. **Archive historical data** for long-term trend analysis
6. **Document system specs** in benchmark results
7. **Run on idle systems** when possible (limit concurrent actions)

## Example: Complete Setup

```bash
# 1. Enable GitHub Pages
./scripts/setup_github_pages.sh

# 2. Commit the benchmark workflow (already done)
git add .github/workflows/benchmarks.yml
git commit -m "Add automated benchmarking workflow"
git push origin master

# 3. Wait for first run
# Check: https://github.com/<username>/fastcond/actions

# 4. View results
# Visit: https://<username>.github.io/fastcond/

# 5. Create a PR to test regression detection
git checkout -b test-performance
# Make changes
git commit -m "Test performance changes"
git push origin test-performance
# Create PR and check for performance comment
```

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub Pages Documentation](https://docs.github.com/en/pages)
- [uv Documentation](https://docs.astral.sh/uv/)
- [Matplotlib Gallery](https://matplotlib.org/stable/gallery/)
- [Bencher.dev](https://bencher.dev/) - Continuous benchmarking platform
