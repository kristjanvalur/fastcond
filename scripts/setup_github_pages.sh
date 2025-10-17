#!/bin/bash
#
# Setup GitHub Pages for benchmark results
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Setting up GitHub Pages for benchmark results..."
echo ""

# Check if gh-pages branch exists
if git show-ref --verify --quiet refs/heads/gh-pages; then
    echo "✓ gh-pages branch already exists"
else
    echo "Creating gh-pages branch..."
    git checkout --orphan gh-pages
    git rm -rf .
    
    # Create initial index
    cat > index.html << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>fastcond Performance Benchmarks</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            line-height: 1.6;
            color: #333;
        }
        h1 {
            color: #2c3e50;
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }
        .info {
            background: #ecf0f1;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .badge {
            display: inline-block;
            padding: 5px 10px;
            background: #3498db;
            color: white;
            border-radius: 3px;
            margin: 5px;
            text-decoration: none;
        }
        .badge:hover {
            background: #2980b9;
        }
    </style>
</head>
<body>
    <h1>🚀 fastcond Performance Benchmarks</h1>
    
    <div class="info">
        <p>This page hosts automated performance benchmark results for the <strong>fastcond</strong> library.</p>
        <p>Benchmarks are run automatically on every commit to master and on pull requests.</p>
    </div>
    
    <h2>Quick Links</h2>
    <a href="https://github.com/kristjanvalur/fastcond" class="badge">GitHub Repository</a>
    <a href="https://github.com/kristjanvalur/fastcond/actions" class="badge">CI Actions</a>
    
    <h2>Latest Results</h2>
    <p>Latest benchmark results will be available here after the first CI run.</p>
    
    <h2>About</h2>
    <p>fastcond provides fast POSIX condition variable alternatives using only semaphores:</p>
    <ul>
        <li><strong>Strong variant</strong>: Full POSIX semantics, 30-40% faster than pthread</li>
        <li><strong>Weak variant</strong>: Relaxed semantics for even better performance</li>
    </ul>
</body>
</html>
EOF
    
    git add index.html
    git commit -m "Initial GitHub Pages setup"
    git push origin gh-pages
    
    git checkout master
    
    echo "✓ gh-pages branch created and pushed"
fi

echo ""
echo "GitHub Pages setup complete!"
echo ""
echo "Next steps:"
echo "1. Enable GitHub Pages in repository settings:"
echo "   Settings → Pages → Source: gh-pages branch"
echo ""
echo "2. Your benchmarks will be available at:"
echo "   https://kristjanvalur.github.io/fastcond/"
echo ""
echo "3. Push a commit to trigger the benchmark workflow"
echo ""
