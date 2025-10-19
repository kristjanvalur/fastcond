#!/usr/bin/env python3
"""
Generate a comprehensive HTML performance page from benchmark results.

This script creates a beautiful HTML page that includes:
- Performance comparison tables
- Interactive charts (PNG images)
- System information
- Navigation and styling
"""

import json
import sys
import argparse
from pathlib import Path
from datetime import datetime

def load_json(filepath):
    """Load benchmark results from JSON file."""
    with open(filepath, 'r') as f:
        return json.load(f)

def generate_html_page(results, output_path, charts_available=True):
    """Generate a comprehensive HTML performance page."""
    
    # Group by benchmark type
    by_benchmark = {}
    system_info = None
    timestamp = None
    
    for result in results:
        if 'error' in result:
            continue
        bench_name = result['benchmark']
        if bench_name not in by_benchmark:
            by_benchmark[bench_name] = []
        by_benchmark[bench_name].append(result)
        
        # Capture system info from first result
        if system_info is None:
            system_info = result.get('system', {})
            timestamp = result.get('timestamp', '')
    
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>fastcond Performance Benchmarks</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, monospace, sans-serif;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            line-height: 1.6;
            color: #333;
            background-color: #fafafa;
        }}
        
        .header {{
            text-align: center;
            background: linear-gradient(135deg, #2c3e50 0%, #3498db 50%, #9b59b6 100%);
            color: white;
            padding: 30px;
            border-radius: 10px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            position: relative;
            overflow: hidden;
        }}
        
        .header::before {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: radial-gradient(circle at 20% 80%, rgba(255,255,255,0.1) 0%, transparent 50%),
                        radial-gradient(circle at 80% 20%, rgba(255,255,255,0.1) 0%, transparent 50%);
            pointer-events: none;
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
        
        .header .tagline {{
            margin: 15px 0 0 0;
            font-size: 1.1em;
            font-weight: 500;
            color: #f39c12;
            text-shadow: 0 1px 2px rgba(0,0,0,0.2);
        }}
        
        .summary-cards {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }}
        
        .card {{
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            border-left: 4px solid #667eea;
        }}
        
        .card h3 {{
            margin: 0 0 10px 0;
            color: #667eea;
            font-size: 1.1em;
        }}
        
        .card .value {{
            font-size: 1.8em;
            font-weight: bold;
            color: #2c3e50;
        }}
        
        .card .label {{
            color: #7f8c8d;
            font-size: 0.9em;
        }}
        
        .benchmark-section {{
            background: white;
            margin-bottom: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            overflow: hidden;
        }}
        
        .benchmark-section h2 {{
            background: #f8f9fa;
            margin: 0;
            padding: 20px;
            border-bottom: 1px solid #e9ecef;
            color: #2c3e50;
        }}
        
        .benchmark-content {{
            padding: 20px;
        }}
        
        .charts-container {{
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin: 20px 0;
        }}
        
        .chart {{
            text-align: center;
            background: #f8f9fa;
            border-radius: 8px;
            padding: 15px;
        }}
        
        .chart img {{
            max-width: 100%;
            height: auto;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        
        .chart h4 {{
            margin: 15px 0 5px 0;
            color: #2c3e50;
        }}
        
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: white;
        }}
        
        th, td {{
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #e9ecef;
        }}
        
        th {{
            background: #f8f9fa;
            font-weight: 600;
            color: #2c3e50;
        }}
        
        .numeric {{
            text-align: right;
            font-family: 'Monaco', 'Menlo', monospace;
        }}
        
        .improvement {{
            color: #27ae60;
            font-weight: bold;
        }}
        
        .regression {{
            color: #e74c3c;
            font-weight: bold;
        }}
        
        .baseline {{
            color: #7f8c8d;
            font-style: italic;
        }}
        
        .system-info {{
            background: #f8f9fa;
            padding: 15px;
            border-radius: 6px;
            margin: 20px 0;
            font-size: 0.9em;
            color: #6c757d;
        }}
        
        .footer {{
            text-align: center;
            margin-top: 40px;
            padding: 20px;
            color: #6c757d;
            border-top: 1px solid #e9ecef;
        }}
        
        .nav-links {{
            text-align: center;
            margin: 20px 0;
        }}
        
        .nav-links a {{
            display: inline-block;
            margin: 0 10px;
            padding: 8px 16px;
            background: #667eea;
            color: white;
            text-decoration: none;
            border-radius: 4px;
            transition: background 0.3s;
        }}
        
        .nav-links a:hover {{
            background: #5a6fd8;
        }}
        
        .intro-section {{
            margin: 30px 0;
        }}
        
        .intro-card {{
            background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
            padding: 30px;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            border-left: 5px solid #28a745;
        }}
        
        .intro-card h2 {{
            margin-top: 0;
            color: #2c3e50;
            font-size: 1.8em;
        }}
        
        .intro-card ul {{
            background: white;
            padding: 15px 20px;
            border-radius: 8px;
            border-left: 3px solid #007bff;
        }}
        
        .intro-card code {{
            background: #f1f3f4;
            padding: 2px 6px;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            color: #d63384;
        }}
        
        @media (max-width: 768px) {{
            .charts-container {{
                grid-template-columns: 1fr;
            }}
            
            .header h1 {{
                font-size: 2em;
            }}
            
            .summary-cards {{
                grid-template-columns: 1fr;
            }}
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>❄️ fastcond: Nordic Engineering Excellence</h1>
        <div class="subtitle">Forged during Iceland's long winter nights when there's proper time for benchmarking. Fast threads mean more time for stargazing and rum-spiked cocoa.</div>
        <div class="tagline">⚡ 38% faster • 🎯 26% lower latency • ✨ More time for life's pleasures</div>
    </div>
    
    <div class="nav-links">
        <a href="https://github.com/kristjanvalur/fastcond">⭐ Star the Repo</a>
        <a href="gil-fairness.html">🎭 GIL Fairness Analysis</a>
        <a href="benchmark-results.json">📊 Raw Data</a>
        <a href="../..">🔄 Latest Results</a>
        <a href="https://github.com/kristjanvalur/fastcond/releases">📦 Get fastcond</a>
    </div>
    
    <div class="intro-section">
        <div class="intro-card">
            <h2>❄️ Why fastcond?</h2>
            <p>Because life is too precious to waste on sluggish thread synchronisation. Built in Iceland's long winter nights, 
            fastcond understands that your threads should work efficiently so <em>you</em> can enjoy the important things: 
            reading poetry under starry skies, sipping rum-spiked cocoa by the fireplace, or watching the aurora dance across the darkness.</p>
            
            <p>Built using nothing but POSIX semaphores and Nordic engineering sensibilities, 
            fastcond delivers performance that lets you spend less time debugging race conditions and more time living.</p>
            
            <p><strong>Two varieties of excellence:</strong></p>
            <ul>
                <li><strong>💪 Strong</strong> - Strict POSIX semantics with surprisingly better performance</li>
                <li><strong>🏃 Weak</strong> - Relaxed semantics for when you're feeling adventurous</li>
            </ul>
            
            <p><em>Rather amusing fact: Both are faster than pthread. We've checked. Thoroughly. During those long Nordic nights when there's proper time for benchmarking.</em></p>
        </div>
    </div>
"""

    # Add summary cards
    if by_benchmark:
        # Calculate overall improvements
        improvements = []
        for bench_name, bench_results in by_benchmark.items():
            pthread_throughput = None
            fastcond_throughput = None
            
            for result in bench_results:
                impl = result['implementation']
                throughput = result['results']['overall'].get('throughput_items_per_sec', 0)
                
                if impl == 'pthread':
                    pthread_throughput = throughput
                elif impl == 'fastcond_strong':
                    fastcond_throughput = throughput
            
            if pthread_throughput and fastcond_throughput:
                improvement = ((fastcond_throughput / pthread_throughput) - 1) * 100
                improvements.append(improvement)
        
        avg_improvement = sum(improvements) / len(improvements) if improvements else 0
        max_improvement = max(improvements) if improvements else 0
        
        html_content += f"""
    <div class="summary-cards">
        <div class="card">
            <h3>Average Performance Gain</h3>
            <div class="value">{avg_improvement:+.1f}%</div>
            <div class="label">vs pthread condition variables</div>
        </div>
        <div class="card">
            <h3>Best Case Improvement</h3>
            <div class="value">{max_improvement:+.1f}%</div>
            <div class="label">maximum observed speedup</div>
        </div>
        <div class="card">
            <h3>Test Configurations</h3>
            <div class="value">{len(by_benchmark)}</div>
            <div class="label">different benchmark scenarios</div>
        </div>
        <div class="card">
            <h3>System</h3>
            <div class="value">{system_info.get('os', 'Unknown')}</div>
            <div class="label">{system_info.get('arch', '')}, {system_info.get('cores', 'N/A')} cores</div>
        </div>
    </div>
"""

    # Add charts section if available
    if charts_available:
        html_content += """
    <div class="benchmark-section">
        <h2>📊 Performance Comparison Charts</h2>
        <div class="benchmark-content">
            <div class="charts-container">
                <div class="chart">
                    <h4>Throughput Comparison</h4>
                    <img src="throughput-comparison.png" alt="Throughput Comparison Chart">
                    <p>Items processed per second across different implementations</p>
                </div>
                <div class="chart">
                    <h4>Latency Comparison</h4>
                    <img src="latency-comparison.png" alt="Latency Comparison Chart">
                    <p>Average message latency with error bars</p>
                </div>
            </div>
        </div>
    </div>
"""

    # Add detailed results for each benchmark
    for bench_name, bench_results in sorted(by_benchmark.items()):
        if not bench_results:
            continue
            
        config = bench_results[0].get('description', f'{bench_name} benchmark')
        
        html_content += f"""
    <div class="benchmark-section">
        <h2>{bench_name.upper()} Results</h2>
        <div class="benchmark-content">
            <p><strong>Configuration:</strong> {config}</p>
            
            <h3>Throughput Comparison</h3>
            <table>
                <thead>
                    <tr>
                        <th>Implementation</th>
                        <th class="numeric">Throughput (items/sec)</th>
                        <th class="numeric">Speedup vs pthread</th>
                    </tr>
                </thead>
                <tbody>
"""
        
        # Find pthread baseline
        pthread_throughput = None
        for result in bench_results:
            if result['implementation'] == 'pthread':
                pthread_throughput = result['results']['overall'].get('throughput_items_per_sec', 0)
                break
        
        # Sort by throughput
        sorted_results = sorted(bench_results, key=lambda x: x['results']['overall'].get('throughput_items_per_sec', 0), reverse=True)
        
        for result in sorted_results:
            impl = result['implementation']
            throughput = result['results']['overall'].get('throughput_items_per_sec', 0)
            
            speedup_class = ""
            speedup_text = ""
            
            if pthread_throughput and impl != 'pthread':
                speedup_ratio = (throughput / pthread_throughput - 1) * 100
                if speedup_ratio > 2:
                    speedup_class = "improvement"
                    speedup_text = f"+{speedup_ratio:.1f}%"
                elif speedup_ratio < -2:
                    speedup_class = "regression"
                    speedup_text = f"{speedup_ratio:.1f}%"
                else:
                    speedup_text = f"{speedup_ratio:+.1f}%"
            elif impl == 'pthread':
                speedup_class = "baseline"
                speedup_text = "baseline"
            
            html_content += f"""
                    <tr>
                        <td>{impl}</td>
                        <td class="numeric">{throughput:,.2f}</td>
                        <td class="numeric {speedup_class}">{speedup_text}</td>
                    </tr>"""
        
        html_content += """
                </tbody>
            </table>
            
            <h3>Latency Statistics</h3>
            <table>
                <thead>
                    <tr>
                        <th>Implementation</th>
                        <th class="numeric">Avg Latency (μs)</th>
                        <th class="numeric">Min (μs)</th>
                        <th class="numeric">Max (μs)</th>
                        <th class="numeric">Std Dev (μs)</th>
                    </tr>
                </thead>
                <tbody>
"""
        
        for result in sorted(bench_results, key=lambda x: x['implementation']):
            impl = result['implementation']
            threads = result['results']['per_thread']
            
            if not threads:
                continue
            
            # Calculate weighted average latency
            total_items = sum(t['items'] for t in threads)
            weighted_avg = sum(t['avg_latency_sec'] * t['items'] for t in threads) / total_items if total_items > 0 else 0
            
            # Get min/max across all threads
            min_lat = min(t['min_latency_sec'] for t in threads)
            max_lat = max(t['max_latency_sec'] for t in threads)
            
            # Average stdev
            avg_stdev = sum(t['stdev_latency_sec'] for t in threads) / len(threads)
            
            html_content += f"""
                    <tr>
                        <td>{impl}</td>
                        <td class="numeric">{weighted_avg*1e6:.2f}</td>
                        <td class="numeric">{min_lat*1e6:.2f}</td>
                        <td class="numeric">{max_lat*1e6:.2f}</td>
                        <td class="numeric">{avg_stdev*1e6:.2f}</td>
                    </tr>"""
        
        html_content += """
                </tbody>
            </table>
        </div>
    </div>
"""

    # Add system information and footer
    if timestamp:
        formatted_time = datetime.fromisoformat(timestamp.replace('Z', '+00:00')).strftime('%B %d, %Y at %H:%M UTC')
    else:
        formatted_time = 'Unknown'
    
    html_content += f"""
    <div class="system-info">
        <strong>System Information:</strong>
        OS: {system_info.get('os', 'Unknown')} {system_info.get('arch', '')}, 
        Cores: {system_info.get('cores', 'N/A')}, 
        Python: {system_info.get('python_version', 'N/A')}<br>
        <strong>Generated:</strong> {formatted_time}
    </div>
    
    <div class="footer">
        <p>❄️ <strong>fastcond</strong> - Forged in Iceland's winter nights so you can spend less time synchronising and more time living</p>
        <p>Built with semaphores, Nordic pragmatism, and the understanding that efficient code means more time for poetry and cocoa</p>
        <p>Why waste precious moments on sluggish pthread when you could be watching stars? Your threads will work brilliantly whilst you enjoy life's finer pleasures. ✨</p>
        <p><a href="https://github.com/kristjanvalur/fastcond">Star us on GitHub</a> | 
           <a href="benchmark-results.json">Download the Evidence</a> | 
           <a href="https://github.com/kristjanvalur/fastcond/issues/new">Share your cocoa-fuelled thoughts</a></p>
    </div>
    
</body>
</html>"""
    
    with open(output_path, 'w') as f:
        f.write(html_content)

def main():
    parser = argparse.ArgumentParser(description='Generate HTML performance page from JSON results')
    parser.add_argument('json_file', help='JSON file with benchmark results')
    parser.add_argument('--output', '-o', default='index.html', help='Output HTML file')
    parser.add_argument('--no-charts', action='store_true', help='Skip chart references')
    
    args = parser.parse_args()
    
    # Load data
    results = load_json(args.json_file)
    
    # Generate HTML page
    generate_html_page(results, args.output, charts_available=not args.no_charts)
    print(f"HTML performance page generated: {args.output}")

if __name__ == '__main__':
    main()