#!/usr/bin/env python3
"""
Generate a comprehensive HTML page for GIL fairness analysis.

Creates a dedicated page showcasing the dramatic differences between
FAIR, UNFAIR, and NAIVE GIL implementations with focus on fairness metrics.
"""

import json
import sys
import argparse
from pathlib import Path
from datetime import datetime

def load_benchmark_data(json_file):
    """Load and filter GIL test results from benchmark JSON."""
    with open(json_file, 'r') as f:
        data = json.load(f)
    
    # Filter for GIL tests only
    gil_results = [result for result in data if result.get('benchmark') == 'gil_test']
    
    if not gil_results:
        raise ValueError("No GIL test results found in benchmark data")
    
    return gil_results

def extract_fairness_data(results):
    """Extract and organize fairness data by mode."""
    modes = {'fair': None, 'unfair': None, 'naive': None}
    
    for result in results:
        fairness_mode = result.get('fairness_mode')
        if fairness_mode in modes:
            fairness_stats = result.get('results', {}).get('fairness_stats', {})
            overall_stats = result.get('results', {}).get('overall', {})
            
            modes[fairness_mode] = {
                'coefficient_of_variation': fairness_stats.get('coefficient_of_variation', 0),
                'transition_percentage': fairness_stats.get('transition_percentage', 0),
                'consecutive_percentage': fairness_stats.get('consecutive_percentage', 0),
                'fairness_score': fairness_stats.get('fairness_score', 0),
                'operations_per_sec': overall_stats.get('operations_per_sec', 0),
                'avg_latency_us': overall_stats.get('avg_latency_us', 0),
                'thread_acquisitions': fairness_stats.get('thread_acquisitions', []),
                'implementation': result.get('implementation', ''),
                'timestamp': result.get('timestamp', ''),
                'system': result.get('system', {})
            }
    
    return modes

def generate_gil_fairness_html(modes, output_path):
    """Generate comprehensive GIL fairness analysis HTML page."""
    
    fair, unfair, naive = modes['fair'], modes['unfair'], modes['naive']
    
    # Use the most recent timestamp
    timestamps = [m['timestamp'] for m in modes.values() if m and m['timestamp']]
    if timestamps:
        latest_timestamp = max(timestamps)
        formatted_time = datetime.fromisoformat(latest_timestamp.replace('Z', '+00:00')).strftime('%B %d, %Y at %H:%M UTC')
    else:
        formatted_time = 'Unknown'
    
    # Get system info from first available result
    system_info = next((m['system'] for m in modes.values() if m and m['system']), {})
    
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>fastcond GIL Fairness Analysis</title>
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
            background: linear-gradient(135deg, #e74c3c 0%, #f39c12 50%, #27ae60 100%);
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
            color: #fff;
            text-shadow: 0 1px 2px rgba(0,0,0,0.2);
        }}
        
        .comparison-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }}
        
        .mode-card {{
            background: white;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            overflow: hidden;
            border-top: 5px solid;
        }}
        
        .mode-fair {{ border-top-color: #27ae60; }}
        .mode-unfair {{ border-top-color: #f39c12; }}
        .mode-naive {{ border-top-color: #e74c3c; }}
        
        .mode-header {{
            padding: 20px;
            text-align: center;
        }}
        
        .mode-fair .mode-header {{ background: linear-gradient(135deg, #27ae60, #2ecc71); color: white; }}
        .mode-unfair .mode-header {{ background: linear-gradient(135deg, #f39c12, #e67e22); color: white; }}
        .mode-naive .mode-header {{ background: linear-gradient(135deg, #e74c3c, #c0392b); color: white; }}
        
        .mode-header h2 {{
            margin: 0;
            font-size: 1.8em;
        }}
        
        .mode-header .description {{
            margin: 10px 0 0 0;
            opacity: 0.9;
            font-size: 0.9em;
        }}
        
        .mode-content {{
            padding: 20px;
        }}
        
        .metric-row {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 8px 0;
            border-bottom: 1px solid #f0f0f0;
        }}
        
        .metric-row:last-child {{
            border-bottom: none;
        }}
        
        .metric-label {{
            font-weight: 500;
            color: #555;
        }}
        
        .metric-value {{
            font-family: 'Monaco', 'Menlo', monospace;
            font-weight: bold;
        }}
        
        .metric-excellent {{ color: #27ae60; }}
        .metric-poor {{ color: #e74c3c; }}
        .metric-terrible {{ color: #8b0000; }}
        
        .acquisition-chart {{
            background: #f8f9fa;
            border-radius: 8px;
            padding: 15px;
            margin: 15px 0;
        }}
        
        .acquisition-bar {{
            display: flex;
            height: 30px;
            border-radius: 4px;
            overflow: hidden;
            margin: 10px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        
        .thread-segment {{
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
            font-size: 0.8em;
            min-width: 30px;
        }}
        
        .thread-0 {{ background: #3498db; }}
        .thread-1 {{ background: #e74c3c; }}
        .thread-2 {{ background: #f39c12; }}
        .thread-3 {{ background: #27ae60; }}
        
        .summary-section {{
            background: white;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            padding: 30px;
            margin: 30px 0;
        }}
        
        .insights-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }}
        
        .insight-card {{
            background: #f8f9fa;
            border-radius: 8px;
            padding: 20px;
            border-left: 4px solid #667eea;
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
        
        @media (max-width: 768px) {{
            .comparison-grid {{
                grid-template-columns: 1fr;
            }}
            
            .header h1 {{
                font-size: 2em;
            }}
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>🎭 GIL Fairness Analysis</h1>
        <div class="subtitle">Demonstrating the critical importance of anti-greedy thread scheduling</div>
        <div class="tagline">Why fairness matters: Preventing thread starvation with minimal performance cost</div>
    </div>
    
    <div class="nav-links">
        <a href="index.html">📊 Performance Benchmarks</a>
        <a href="https://github.com/kristjanvalur/fastcond">⭐ GitHub Repository</a>
        <a href="gil-fairness-results.json">📊 Raw Data</a>
    </div>
"""

    # Add the comparison grid
    html_content += """
    <div class="comparison-grid">
"""

    # Generate cards for each mode
    mode_info = {
        'fair': {
            'title': '💪 FAIR Mode',
            'description': 'Anti-greedy fairness mechanism prevents thread starvation',
            'class': 'mode-fair'
        },
        'unfair': {
            'title': '🏃 UNFAIR Mode', 
            'description': 'Condition variables without fairness - allows greedy re-acquisition',
            'class': 'mode-unfair'
        },
        'naive': {
            'title': '🔒 NAIVE Mode',
            'description': 'Simple mutex - no condition variables or fairness logic',
            'class': 'mode-naive'
        }
    }
    
    for mode_key, mode_data in modes.items():
        if not mode_data:
            continue
            
        info = mode_info[mode_key]
        
        # Create acquisition visualization
        acquisitions = mode_data['thread_acquisitions']
        total_acquisitions = sum(t['acquisitions'] for t in acquisitions) if acquisitions else 1
        
        html_content += f"""
        <div class="mode-card {info['class']}">
            <div class="mode-header">
                <h2>{info['title']}</h2>
                <div class="description">{info['description']}</div>
            </div>
            <div class="mode-content">
"""

        # Add acquisition visualization
        if acquisitions:
            html_content += """
                <div class="acquisition-chart">
                    <h4>Thread Acquisition Distribution</h4>
                    <div class="acquisition-bar">
"""
            for i, thread in enumerate(acquisitions):
                percentage = (thread['acquisitions'] / total_acquisitions) * 100
                if percentage > 0:
                    html_content += f"""
                        <div class="thread-segment thread-{i}" style="flex: {percentage}">
                            T{i}: {thread['acquisitions']}
                        </div>
"""
            html_content += """
                    </div>
                </div>
"""

        # Add metrics
        cv = mode_data['coefficient_of_variation']
        cv_class = 'metric-excellent' if cv < 0.1 else 'metric-poor' if cv < 1.0 else 'metric-terrible'
        
        fairness_score = mode_data['fairness_score']
        fs_class = 'metric-excellent' if fairness_score < 10 else 'metric-poor' if fairness_score < 100 else 'metric-terrible'
        
        html_content += f"""
                <div class="metric-row">
                    <span class="metric-label">Coefficient of Variation</span>
                    <span class="metric-value {cv_class}">{cv:.3f}</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Fairness Score</span>
                    <span class="metric-value {fs_class}">{fairness_score:.1f}</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Thread Transitions</span>
                    <span class="metric-value">{mode_data['transition_percentage']:.1f}%</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Consecutive Re-acquisitions</span>
                    <span class="metric-value">{mode_data['consecutive_percentage']:.1f}%</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Operations/sec</span>
                    <span class="metric-value">{mode_data['operations_per_sec']:,}</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Avg Latency</span>
                    <span class="metric-value">{mode_data['avg_latency_us']:.1f} μs</span>
                </div>
            </div>
        </div>
"""

    html_content += """
    </div>
"""

    # Add summary section
    if fair and unfair and naive:
        html_content += f"""
    <div class="summary-section">
        <h2>🔍 Analysis Summary</h2>
        
        <div class="insights-grid">
            <div class="insight-card">
                <h3>Fairness Impact</h3>
                <p><strong>FAIR mode</strong> achieves perfect thread distribution (CV: {fair['coefficient_of_variation']:.3f}) 
                while <strong>UNFAIR</strong> and <strong>NAIVE</strong> modes show severe thread starvation 
                (CV: {unfair['coefficient_of_variation']:.3f} and {naive['coefficient_of_variation']:.3f} respectively).</p>
            </div>
            
            <div class="insight-card">
                <h3>Performance Cost</h3>
                <p>The fairness mechanism adds minimal overhead: 
                <strong>{((unfair['operations_per_sec'] / fair['operations_per_sec'] - 1) * 100):+.1f}%</strong> 
                performance difference between UNFAIR and FAIR modes.</p>
            </div>
            
            <div class="insight-card">
                <h3>Thread Starvation Prevention</h3>
                <p>FAIR mode ensures <strong>{fair['transition_percentage']:.0f}% thread transitions</strong> 
                vs <strong>{unfair['transition_percentage']:.0f}%</strong> in UNFAIR mode, 
                preventing any thread from monopolizing the GIL.</p>
            </div>
            
            <div class="insight-card">
                <h3>Implementation Validation</h3>
                <p>NAIVE and UNFAIR modes show nearly identical behavior 
                (CV: {naive['coefficient_of_variation']:.3f} vs {unfair['coefficient_of_variation']:.3f}), 
                validating that the condition variable machinery without fairness behaves like a simple mutex.</p>
            </div>
        </div>
        
        <h3>Key Takeaway</h3>
        <p>The results demonstrate that <strong>thread fairness is not automatic</strong> - it requires intentional design. 
        The fastcond GIL's anti-greedy mechanism provides dramatic fairness improvements with minimal performance cost, 
        making it essential for any application where thread starvation could impact user experience or system stability.</p>
    </div>
"""

    # Add system info and footer
    html_content += f"""
    <div class="system-info">
        <strong>System Information:</strong>
        OS: {system_info.get('os', 'Unknown')} {system_info.get('arch', '')}, 
        Cores: {system_info.get('cores', 'N/A')}, 
        Python: {system_info.get('python_version', 'N/A')}<br>
        <strong>Generated:</strong> {formatted_time}
    </div>
    
    <div class="footer">
        <p>🎭 <strong>fastcond GIL Fairness Analysis</strong> - Demonstrating why anti-greedy scheduling matters</p>
        <p>Thread fairness isn't accidental - it's engineered. These results show the dramatic difference between 
        naive mutex behavior and intentionally fair thread scheduling.</p>
        <p><a href="https://github.com/kristjanvalur/fastcond">Explore the implementation</a> | 
           <a href="gil-fairness-results.json">Download the data</a> | 
           <a href="index.html">View performance benchmarks</a></p>
    </div>
    
</body>
</html>"""
    
    with open(output_path, 'w') as f:
        f.write(html_content)

def main():
    parser = argparse.ArgumentParser(description='Generate GIL fairness analysis HTML page')
    parser.add_argument('json_file', help='JSON file with GIL benchmark results')
    parser.add_argument('--output', '-o', default='gil-fairness.html', help='Output HTML file')
    
    args = parser.parse_args()
    
    try:
        # Load and process data
        results = load_benchmark_data(args.json_file)
        modes = extract_fairness_data(results)
        
        # Check if we have all required modes
        missing_modes = [mode for mode, data in modes.items() if data is None]
        if missing_modes:
            print(f"Warning: Missing data for modes: {missing_modes}", file=sys.stderr)
        
        # Generate HTML page
        generate_gil_fairness_html(modes, args.output)
        print(f"GIL fairness analysis page generated: {args.output}")
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()