#!/usr/bin/env python3
"""
Visualize performance benchmarks from JSON data.

Creates comparison charts and tables showing pthread vs fastcond performance.
"""

import json
import sys
import argparse
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import matplotlib
    matplotlib.use('Agg')  # Non-interactive backend
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("Warning: matplotlib not available, skipping chart generation", file=sys.stderr)

def load_json(filepath):
    """Load benchmark results from JSON file."""
    with open(filepath, 'r') as f:
        return json.load(f)

def create_comparison_table(results):
    """Create a markdown table comparing implementations."""
    
    # Group by benchmark type
    by_benchmark = {}
    for result in results:
        if 'error' in result:
            continue
        bench_name = result['benchmark']
        if bench_name not in by_benchmark:
            by_benchmark[bench_name] = []
        by_benchmark[bench_name].append(result)
    
    output = []
    
    for bench_name, bench_results in by_benchmark.items():
        output.append(f"\n## {bench_name.upper()}\n")
        
        # Get description
        if bench_results:
            output.append(f"**Configuration:** {bench_results[0].get('description', 'N/A')}\n")
        
        # Throughput comparison
        output.append("### Throughput Comparison\n")
        output.append("| Implementation | Throughput (items/sec) | Speedup vs pthread |\n")
        output.append("|---|---:|---:|\n")
        
        pthread_throughput = None
        throughput_data = []
        
        for result in bench_results:
            impl = result['implementation']
            throughput = result['results']['overall'].get('throughput_items_per_sec', 0)
            
            if impl == 'pthread':
                pthread_throughput = throughput
            
            throughput_data.append((impl, throughput))
        
        for impl, throughput in sorted(throughput_data, key=lambda x: x[1], reverse=True):
            speedup = ""
            if pthread_throughput and impl != 'pthread':
                speedup_ratio = (throughput / pthread_throughput - 1) * 100
                speedup = f"{speedup_ratio:+.1f}%"
            elif impl == 'pthread':
                speedup = "baseline"
            
            output.append(f"| {impl} | {throughput:,.2f} | {speedup} |\n")
        
        # Latency comparison (average across all threads)
        output.append("\n### Average Latency Comparison\n")
        output.append("| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |\n")
        output.append("|---|---:|---:|---:|---:|\n")
        
        for result in bench_results:
            impl = result['implementation']
            threads = result['results'].get('per_thread', [])
            
            if not threads:
                continue
            
            # Calculate overall average
            total_items = sum(t['items'] for t in threads)
            weighted_avg = sum(t['avg_latency_sec'] * t['items'] for t in threads) / total_items if total_items > 0 else 0
            
            # Get min/max across all threads
            min_lat = min(t['min_latency_sec'] for t in threads)
            max_lat = max(t['max_latency_sec'] for t in threads)
            
            # Average stdev
            avg_stdev = sum(t['stdev_latency_sec'] for t in threads) / len(threads)
            
            output.append(f"| {impl} | {weighted_avg*1e6:.2f} | {min_lat*1e6:.2f} | {max_lat*1e6:.2f} | {avg_stdev*1e6:.2f} |\n")
    
    return "".join(output)

def create_throughput_chart(results, output_path):
    """Create bar chart comparing throughput."""
    if not HAS_MATPLOTLIB:
        return
    
    # Group by benchmark
    by_benchmark = {}
    for result in results:
        if 'error' in result:
            continue
        bench_name = result['benchmark']
        if bench_name not in by_benchmark:
            by_benchmark[bench_name] = []
        by_benchmark[bench_name].append(result)
    
    fig, axes = plt.subplots(1, len(by_benchmark), figsize=(6*len(by_benchmark), 5))
    if len(by_benchmark) == 1:
        axes = [axes]
    
    for idx, (bench_name, bench_results) in enumerate(sorted(by_benchmark.items())):
        ax = axes[idx]
        
        implementations = []
        throughputs = []
        colors = []
        
        color_map = {
            'pthread': '#3498db',
            'fastcond_strong': '#2ecc71',
            'fastcond_weak': '#f39c12'
        }
        
        for result in sorted(bench_results, key=lambda x: x['results']['overall'].get('throughput_items_per_sec', 0)):
            impl = result['implementation']
            throughput = result['results']['overall'].get('throughput_items_per_sec', 0)
            
            implementations.append(impl)
            throughputs.append(throughput / 1000)  # Convert to K items/sec
            colors.append(color_map.get(impl, '#95a5a6'))
        
        bars = ax.barh(implementations, throughputs, color=colors)
        
        # Add value labels
        for bar in bars:
            width = bar.get_width()
            ax.text(width, bar.get_y() + bar.get_height()/2, 
                   f'{width:.0f}K',
                   ha='left', va='center', fontsize=10, fontweight='bold')
        
        ax.set_xlabel('Throughput (K items/sec)', fontsize=11, fontweight='bold')
        ax.set_title(f'{bench_name.upper()} Throughput', fontsize=13, fontweight='bold')
        ax.grid(axis='x', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"Chart saved to {output_path}")

def create_latency_chart(results, output_path):
    """Create bar chart comparing average latency."""
    if not HAS_MATPLOTLIB:
        return
    
    # Group by benchmark
    by_benchmark = {}
    for result in results:
        if 'error' in result:
            continue
        bench_name = result['benchmark']
        if bench_name not in by_benchmark:
            by_benchmark[bench_name] = []
        by_benchmark[bench_name].append(result)
    
    fig, axes = plt.subplots(1, len(by_benchmark), figsize=(6*len(by_benchmark), 5))
    if len(by_benchmark) == 1:
        axes = [axes]
    
    for idx, (bench_name, bench_results) in enumerate(sorted(by_benchmark.items())):
        ax = axes[idx]
        
        implementations = []
        latencies = []
        errors = []
        colors = []
        
        color_map = {
            'pthread': '#3498db',
            'fastcond_strong': '#2ecc71',
            'fastcond_weak': '#f39c12'
        }
        
        for result in sorted(bench_results, key=lambda x: x['implementation']):
            impl = result['implementation']
            threads = result['results'].get('per_thread', [])
            
            if not threads:
                continue
            
            # Calculate weighted average latency
            total_items = sum(t['items'] for t in threads)
            weighted_avg = sum(t['avg_latency_sec'] * t['items'] for t in threads) / total_items if total_items > 0 else 0
            avg_stdev = sum(t['stdev_latency_sec'] for t in threads) / len(threads)
            
            implementations.append(impl)
            latencies.append(weighted_avg * 1e6)  # Convert to microseconds
            errors.append(avg_stdev * 1e6)
            colors.append(color_map.get(impl, '#95a5a6'))
        
        bars = ax.barh(implementations, latencies, xerr=errors, color=colors, capsize=5)
        
        # Add value labels
        for bar, err in zip(bars, errors):
            width = bar.get_width()
            ax.text(width + err, bar.get_y() + bar.get_height()/2,
                   f'{width:.1f}μs',
                   ha='left', va='center', fontsize=10, fontweight='bold')
        
        ax.set_xlabel('Average Latency (μs)', fontsize=11, fontweight='bold')
        ax.set_title(f'{bench_name.upper()} Latency', fontsize=13, fontweight='bold')
        ax.grid(axis='x', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"Chart saved to {output_path}")

def main():
    parser = argparse.ArgumentParser(description='Visualize fastcond performance benchmarks')
    parser.add_argument('json_file', help='JSON file with benchmark results')
    parser.add_argument('--output-dir', '-o', default='docs', help='Output directory for charts and tables')
    parser.add_argument('--no-charts', action='store_true', help='Skip chart generation')
    
    args = parser.parse_args()
    
    # Load data
    results = load_json(args.json_file)
    
    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    
    # Generate comparison table
    table = create_comparison_table(results)
    table_file = output_dir / 'benchmark-results.md'
    with open(table_file, 'w') as f:
        f.write("# Performance Benchmark Results\n")
        f.write(f"\nGenerated from: `{args.json_file}`\n")
        f.write(table)
    print(f"Table saved to {table_file}")
    
    # Generate charts if matplotlib is available
    if not args.no_charts and HAS_MATPLOTLIB:
        create_throughput_chart(results, output_dir / 'throughput-comparison.png')
        create_latency_chart(results, output_dir / 'latency-comparison.png')
    elif not HAS_MATPLOTLIB:
        print("Install matplotlib to generate charts: pip install matplotlib")

if __name__ == '__main__':
    main()
