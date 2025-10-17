# Development Documentation

This directory contains documentation for developers and contributors to the fastcond project.

## Overview

- **docs/** - User-facing documentation (API, performance results, etc.)
- **dev-docs/** - Development and process documentation (this directory)

## Contents

### Benchmarking & Performance
- **[BENCHMARKS-QUICKSTART.md](BENCHMARKS-QUICKSTART.md)** - Quick reference for all benchmarking usage modes
- **[benchmarking.md](benchmarking.md)** - Complete local benchmarking guide
- **[ci-benchmarks.md](ci-benchmarks.md)** - Complete CI/CD integration guide

### Deployment & Process
- **[DEPLOY-NOW.md](DEPLOY-NOW.md)** - Simple step-by-step deployment checklist
- **[DEPLOYMENT.md](DEPLOYMENT.md)** - Comprehensive deployment guide
- **[test-hanging-issue.md](test-hanging-issue.md)** - Documentation of macOS hanging issue resolution

## Quick Navigation

### For Contributors
- Setting up benchmarks: [BENCHMARKS-QUICKSTART.md](BENCHMARKS-QUICKSTART.md)
- CI/CD workflow: [ci-benchmarks.md](ci-benchmarks.md)
- Deployment process: [DEPLOY-NOW.md](DEPLOY-NOW.md)

### For Maintainers
- Complete deployment guide: [DEPLOYMENT.md](DEPLOYMENT.md)
- Benchmarking system: [benchmarking.md](benchmarking.md)
- Historical issues: [test-hanging-issue.md](test-hanging-issue.md)

## Documentation Structure

```
fastcond/
├── docs/                           # User-facing documentation
│   ├── benchmark-results.json      # Latest benchmark data
│   ├── benchmark-results.md        # Latest benchmark tables
│   ├── performance-analysis.md     # Performance insights
│   └── *.png                      # Benchmark charts
│
└── dev-docs/                      # Development documentation (this dir)
    ├── README.md                   # This file
    ├── BENCHMARKS-QUICKSTART.md    # Quick benchmarking reference
    ├── benchmarking.md             # Complete benchmarking guide
    ├── ci-benchmarks.md            # CI/CD integration guide
    ├── DEPLOY-NOW.md               # Quick deployment checklist
    ├── DEPLOYMENT.md               # Complete deployment guide
    └── test-hanging-issue.md       # Historical issue documentation
```

## Contributing to Documentation

When adding new documentation:

1. **User-facing docs** → Place in `docs/`
   - API documentation
   - Performance results
   - Usage examples
   - Public benchmarks

2. **Development docs** → Place in `dev-docs/`
   - Build processes
   - CI/CD workflows
   - Deployment guides
   - Troubleshooting
   - Historical issue documentation

3. **Update this README** when adding new files

## Links to Other Documentation

- **[../README.md](../README.md)** - Main project README
- **[../BUILD.md](../BUILD.md)** - Build instructions
- **[../CONTRIBUTING.md](../CONTRIBUTING.md)** - Contribution guidelines
- **[../CHANGELOG.md](../CHANGELOG.md)** - Version history