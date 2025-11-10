#!/usr/bin/env python3
"""
Generate root index.html page with performance summary.
"""

import json
import datetime
import sys
from pathlib import Path


def main():
    if len(sys.argv) != 3:
        print("Usage: generate_root_page.py <performance-summary.json> <output-path>")
        sys.exit(1)

    summary_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2])

    # Load the performance summary
    with open(summary_file, "r") as f:
        data = json.load(f)

    # Calculate overall performance improvements
    improvements = []
    platforms = set()

    for result in data:
        platform = result["platform"]
        test = result["test"]
        speedup = result["speedup_vs_native"]
        platforms.add(platform.title())

        if speedup > 1.0:
            improvements.append(f"{test}: {speedup:.1f}x faster on {platform.title()}")

    platforms_str = ", ".join(sorted(platforms))
    avg_improvement = sum(
        r["speedup_vs_native"] for r in data if r["speedup_vs_native"] > 1.0
    )
    num_improvements = len([r for r in data if r["speedup_vs_native"] > 1.0])
    avg_improvement = avg_improvement / max(num_improvements, 1)

    best_improvement = max((r["speedup_vs_native"] for r in data), default=1.0)

    # Generate timestamp
    timestamp = datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S UTC")

    # Create root index.html
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>fastcond - Fast POSIX Condition Variables</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            line-height: 1.6;
            color: #333;
        }}
        .header {{
            text-align: center;
            background: linear-gradient(135deg, #2c3e50 0%, #3498db 100%);
            color: white;
            padding: 40px 20px;
            border-radius: 12px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }}
        .header h1 {{
            margin: 0;
            font-size: 2.5em;
            font-weight: 300;
        }}
        .header .subtitle {{
            margin: 10px 0 0 0;
            font-size: 1.2em;
            opacity: 0.9;
        }}
        .performance-highlight {{
            background: linear-gradient(135deg, #27ae60, #2ecc71);
            color: white;
            padding: 30px;
            border-radius: 12px;
            margin: 30px 0;
            text-align: center;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }}
        .performance-highlight h2 {{
            margin: 0 0 15px 0;
            font-size: 1.8em;
        }}
        .performance-highlight .metric {{
            font-size: 2.2em;
            font-weight: bold;
            margin: 10px 0;
        }}
        .performance-highlight .details {{
            font-size: 1.1em;
            opacity: 0.9;
        }}
        .features-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }}
        .feature-card {{
            background: white;
            padding: 25px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            border-left: 4px solid #3498db;
        }}
        .feature-card h3 {{
            margin: 0 0 15px 0;
            color: #2c3e50;
        }}
        .cta-section {{
            background: #f8f9fa;
            padding: 30px;
            border-radius: 12px;
            text-align: center;
            margin: 30px 0;
        }}
        .cta-buttons {{
            margin: 20px 0 0 0;
        }}
        .cta-buttons a {{
            display: inline-block;
            margin: 0 10px;
            padding: 12px 24px;
            background: #3498db;
            color: white;
            text-decoration: none;
            border-radius: 6px;
            font-weight: 500;
            transition: background 0.3s;
        }}
        .cta-buttons a:hover {{
            background: #2980b9;
        }}
        .cta-buttons a.primary {{
            background: #27ae60;
        }}
        .cta-buttons a.primary:hover {{
            background: #229954;
        }}
        .footer {{
            margin-top: 50px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            color: #7f8c8d;
            text-align: center;
            font-size: 0.9em;
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>⚡ fastcond</h1>
        <div class="subtitle">Fast POSIX Condition Variables Using Only Semaphores</div>
    </div>
    
    <div class="performance-highlight">
        <h2>🚀 Proven Performance Across Platforms</h2>
        <div class="metric">Up to {best_improvement:.1f}x Faster</div>
        <div class="details">
            Comprehensive benchmarks across {platforms_str} show consistent performance improvements.<br>
            Average improvement: {avg_improvement:.1f}x faster than native implementations.
        </div>
    </div>
    
    <div class="features-grid">
        <div class="feature-card">
            <h3>🎯 Drop-in Replacement</h3>
            <p>Full POSIX condition variable semantics with zero API changes. Replace pthread_cond_* with fastcond equivalents and get immediate performance benefits.</p>
        </div>
        
        <div class="feature-card">
            <h3>🏗️ Two Variants</h3>
            <p><strong>Weak</strong>: Maximum performance with relaxed semantics. <strong>Strong</strong>: Full POSIX compliance with excellent performance.</p>
        </div>
        
        <div class="feature-card">
            <h3>🌍 Cross-Platform</h3>
            <p>Tested and optimized for Linux, macOS, and Windows. Uses platform-appropriate semaphore implementations for best performance.</p>
        </div>
        
        <div class="feature-card">
            <h3>📊 Rigorously Tested</h3>
            <p>Comprehensive test suite with AddressSanitizer, ThreadSanitizer, and statistical analysis. Every commit is benchmarked across platforms.</p>
        </div>
    </div>
    
    <div class="cta-section">
        <h2>Get Started with fastcond</h2>
        <p>Ready to speed up your thread synchronization? Choose your path:</p>
        <div class="cta-buttons">
            <a href="performance/" class="primary">📊 View Detailed Benchmarks</a>
            <a href="https://github.com/kristjanvalur/fastcond">⭐ Star on GitHub</a>
            <a href="https://github.com/kristjanvalur/fastcond/releases">📦 Download Release</a>
            <a href="https://github.com/kristjanvalur/fastcond#readme">📖 Documentation</a>
        </div>
    </div>
    
    <div class="footer">
        <p>Built with semaphores and Nordic engineering precision.</p>
        <p>Last updated: {timestamp}</p>
        <p><a href="https://github.com/kristjanvalur/fastcond">kristjanvalur/fastcond</a></p>
    </div>
</body>
</html>"""

    # Write the HTML file
    with open(output_file, "w") as f:
        f.write(html_content)

    print(f"✅ Generated updated root index page: {output_file}")


if __name__ == "__main__":
    main()
