# Deployment Guide: Pushing Benchmarking System to GitHub

## Current State
✅ You have 4 commits ready to push:
1. Enhanced performance measurements in tests
2. Add performance analysis documentation  
3. Add comprehensive benchmarking and visualization system
4. Add CI/CD benchmarking integration and GitHub Pages publishing

## Step-by-Step Deployment

### Step 1: Push to GitHub
```bash
cd /home/kristjan/git/fastcond

# Push all commits to master
git push origin master
```

**What happens:**
- Your 4 commits go to GitHub
- The CI workflow will be registered
- **BUT** the benchmark workflow won't run yet (it needs the build targets)

### Step 2: Verify CI Workflow Registration
```bash
# Check GitHub Actions tab
# You should see "Performance Benchmarks" workflow listed
```

Or visit: `https://github.com/kristjanvalur/fastcond/actions`

### Step 3: Set Up GitHub Pages (Optional but Recommended)

#### Option A: Using the Script (Recommended)
```bash
# This creates the gh-pages branch and pushes it
./scripts/setup_github_pages.sh
```

#### Option B: Manual Setup
```bash
# Create orphan branch
git checkout --orphan gh-pages
git rm -rf .

# Create basic index
cat > index.html << 'EOF'
<!DOCTYPE html>
<html>
<head><title>fastcond Benchmarks</title></head>
<body>
<h1>fastcond Performance Benchmarks</h1>
<p>Benchmark results will appear here after CI runs.</p>
</body>
</html>
EOF

git add index.html
git commit -m "Initial GitHub Pages setup"
git push origin gh-pages

# Return to master
git checkout master
```

### Step 4: Enable GitHub Pages in Repository Settings

1. Go to: `https://github.com/kristjanvalur/fastcond/settings/pages`
2. Under **Source**, select:
   - Branch: `gh-pages`
   - Folder: `/ (root)`
3. Click **Save**

**Result:** Your benchmarks will be published to:
```
https://kristjanvalur.github.io/fastcond/
```

### Step 5: Trigger First Benchmark Run

#### Option A: Manual Trigger (Immediate)
1. Go to: `https://github.com/kristjanvalur/fastcond/actions`
2. Click "Performance Benchmarks" workflow
3. Click "Run workflow" dropdown
4. Select `master` branch
5. Click green "Run workflow" button

#### Option B: Wait for Next Push
The workflow will automatically run on your next commit to master.

#### Option C: Create a Test PR
```bash
git checkout -b test-benchmarks
echo "# test" >> README.md
git commit -am "Test: trigger benchmark workflow"
git push origin test-benchmarks
```
Then create a PR in GitHub UI - benchmarks will run automatically!

### Step 6: Verify Everything Works

**Check 1: Workflow Runs**
- Go to Actions tab
- Should see "Performance Benchmarks" running or completed
- Green checkmark = success

**Check 2: Artifacts Generated**
- Click on completed workflow run
- Scroll to "Artifacts" section
- Should see `benchmark-results-<commit-sha>.zip`
- Download to verify contents

**Check 3: GitHub Pages (if enabled)**
- Visit: `https://kristjanvalur.github.io/fastcond/`
- Should see benchmark results
- May take 1-2 minutes after workflow completes

**Check 4: PR Comments (if tested with PR)**
- Should see automatic comment with performance comparison
- Includes emoji indicators (🟢🔴⚪)

## Troubleshooting

### Issue: Workflow Doesn't Run

**Cause:** Test executables not built

**Solution:** The workflow builds them automatically, but if it fails:
```bash
cd test
make clean
make all CFLAGS="-O3 -DNDEBUG"
```

Then check the Actions tab for error messages.

### Issue: GitHub Pages Shows 404

**Wait time:** GitHub Pages can take 1-2 minutes to deploy after first push

**Check:**
1. Settings → Pages → Verify `gh-pages` branch is selected
2. Check Actions tab for "pages build and deployment" workflow
3. Ensure `gh-pages` branch exists: `git branch -r | grep gh-pages`

**Fix:**
```bash
# Re-run the setup script
./scripts/setup_github_pages.sh
```

### Issue: uv Not Found in CI

**Should not happen** - the workflow installs uv automatically via:
```yaml
- uses: astral-sh/setup-uv@v5
```

If it fails, check the Actions log for the "Install uv" step.

### Issue: Charts Not Generated in CI

**Cause:** matplotlib not installed

**Check:** The workflow runs `uv sync` which should install matplotlib

**Verify locally:**
```bash
uv sync
uv run python -c "import matplotlib; print(matplotlib.__version__)"
```

## Expected Timeline

| Action | Time | What to Watch |
|--------|------|---------------|
| `git push` | ~10s | GitHub receives commits |
| Workflow triggers | ~30s | Appears in Actions tab |
| Workflow runs | ~3-5min | Build, test, benchmark, deploy |
| Artifacts available | Immediate | After workflow completes |
| GitHub Pages live | ~1-2min | After workflow completes |

## Verification Checklist

After pushing and running workflows:

- [ ] All 4 commits pushed to GitHub
- [ ] "Performance Benchmarks" workflow appears in Actions
- [ ] Workflow runs successfully (green checkmark)
- [ ] Artifacts downloadable from workflow run
- [ ] `gh-pages` branch exists
- [ ] GitHub Pages enabled in settings
- [ ] Benchmark results visible at `https://kristjanvalur.github.io/fastcond/`
- [ ] PR benchmarks work (if tested with PR)
- [ ] Charts render correctly (PNG files)
- [ ] JSON data is well-formed

## Quick Command Reference

```bash
# Push commits
git push origin master

# Setup GitHub Pages
./scripts/setup_github_pages.sh

# Run benchmarks locally
./scripts/run_benchmarks.sh

# Check what will be pushed
git log origin/master..master --oneline

# View workflow status
gh run list  # (if you have GitHub CLI)

# Download latest artifact
gh run download  # (if you have GitHub CLI)
```

## What Happens on First Run

1. **Workflow starts** (~30s after push)
2. **Checkout code** (5s)
3. **Install uv** (10s)
4. **Install build tools** (30s)
5. **Install Python deps** (20s via uv)
6. **Build tests** (30s)
7. **Run benchmarks** (1-2min)
8. **Generate visualizations** (10s)
9. **Upload artifacts** (5s)
10. **Deploy to GitHub Pages** (30s if master)

**Total:** ~3-5 minutes

## After Successful Deployment

You'll have:
- ✅ Automated benchmarks on every commit
- ✅ Performance regression detection on PRs
- ✅ Public benchmark results online
- ✅ Historical performance tracking (weekly)
- ✅ Downloadable artifacts for analysis

## Next Steps After Deployment

1. **Share your benchmark results:**
   ```markdown
   See our performance benchmarks:
   https://kristjanvalur.github.io/fastcond/
   ```

2. **Add badge to README:**
   ```markdown
   [![Benchmarks](https://img.shields.io/badge/benchmarks-passing-brightgreen)](https://kristjanvalur.github.io/fastcond/)
   ```

3. **Monitor performance over time:**
   - Weekly runs will accumulate historical data
   - Download and analyze JSON from different dates

4. **Customize thresholds:**
   - Edit `.github/workflows/benchmarks.yml`
   - Adjust the ±2% threshold for regression detection

## Need Help?

- **CI Issues:** Check Actions tab → Click workflow → View logs
- **Local Issues:** See [docs/benchmarking.md](docs/benchmarking.md)
- **GitHub Pages:** See [docs/ci-benchmarks.md](docs/ci-benchmarks.md)
