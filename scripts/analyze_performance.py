#!/usr/bin/env python3
"""
Performance Analysis Script for fastcond

This script collates performance data from multiple platform artifacts,
performs comparative analysis, and generates visualization charts.

Input: CSV files from CI workflow artifacts (performance-results.csv)
Output: Comparative charts, tables, and analysis reports
"""

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, List, Tuple

try:
    import matplotlib.pyplot as plt
    import matplotlib
    matplotlib.use('Agg')  # Non-interactive backend
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("Warning: matplotlib not available, charts will not be generated")


class PerformanceData:
    """Container for performance benchmark results"""
    
    def __init__(self, platform: str, os_version: str, test: str, variant: str,
                 threads: int, param: int, iterations: int, elapsed_sec: float,
                 throughput: float):
        self.platform = platform
        self.os_version = os_version
        self.test = test
        self.variant = variant
        self.threads = threads
        self.param = param  # queue_size or operations
        self.iterations = iterations
        self.elapsed_sec = elapsed_sec
        self.throughput = throughput
    
    @classmethod
    def from_csv_row(cls, row: Dict[str, str]) -> 'PerformanceData':
        """Parse CSV row into PerformanceData object"""
        return cls(
            platform=row['platform'],
            os_version=row['os_version'],
            test=row['test'],
            variant=row['variant'],
            threads=int(row['threads']),
            param=int(row['param']),
            iterations=int(row['iterations']),
            elapsed_sec=float(row['elapsed_sec']),
            throughput=float(row['throughput'])
        )
    
    def key(self) -> Tuple:
        """Generate unique key for grouping results"""
        return (self.platform, self.test, self.threads, self.param)
    
    def __repr__(self) -> str:
        return (f"PerformanceData({self.platform}, {self.test}, {self.variant}, "
                f"threads={self.threads}, throughput={self.throughput:.2f})")


def load_performance_data(csv_files: List[Path]) -> List[PerformanceData]:
    """Load and merge performance data from multiple CSV files"""
    results = []
    
    for csv_file in csv_files:
        if not csv_file.exists():
            print(f"Warning: File not found: {csv_file}")
            continue
        
        print(f"Loading data from {csv_file}...")
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    data = PerformanceData.from_csv_row(row)
                    results.append(data)
                except (KeyError, ValueError) as e:
                    print(f"Warning: Skipping invalid row in {csv_file}: {e}")
                    continue
    
    print(f"Loaded {len(results)} benchmark results from {len(csv_files)} files")
    return results


def group_by_configuration(data: List[PerformanceData]) -> Dict[Tuple, List[PerformanceData]]:
    """Group results by (platform, test, threads, param)"""
    grouped = defaultdict(list)
    for item in data:
        grouped[item.key()].append(item)
    return dict(grouped)


def calculate_speedup(native_throughput: float, fastcond_throughput: float) -> float:
    """Calculate speedup ratio (fastcond / native)"""
    if native_throughput == 0:
        return 0.0
    return fastcond_throughput / native_throughput


def generate_comparison_table(data: List[PerformanceData]) -> str:
    """Generate markdown comparison table"""
    grouped = group_by_configuration(data)
    
    table = []
    table.append("# Performance Comparison: fastcond vs Native")
    table.append("")
    table.append("| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |")
    table.append("|----------|------|---------|---------|---------------------|-------------------|")
    
    for key in sorted(grouped.keys()):
        platform, test, threads, param = key
        results = grouped[key]
        
        # Find native baseline
        native = next((r for r in results if r.variant == 'native'), None)
        native_throughput = native.throughput if native else 0.0
        
        # Add rows for each variant
        for result in sorted(results, key=lambda x: x.variant):
            speedup = calculate_speedup(native_throughput, result.throughput)
            speedup_str = f"{speedup:.2f}x" if speedup > 0 else "N/A"
            
            table.append(
                f"| {result.platform} | {result.test} | {result.threads} | "
                f"{result.variant} | {result.throughput:,.0f} | {speedup_str} |"
            )
    
    return "\n".join(table)


def generate_charts(data: List[PerformanceData], output_dir: Path):
    """Generate comparative performance charts"""
    if not HAS_MATPLOTLIB:
        print("Skipping chart generation (matplotlib not available)")
        return
    
    grouped = group_by_configuration(data)
    
    # Chart 1: Throughput comparison by platform
    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    fig.suptitle('fastcond Performance Across Platforms', fontsize=16)
    
    platforms = sorted(set(d.platform for d in data))
    tests = ['qtest', 'strongtest', 'gil_test']
    
    for idx, test in enumerate(tests):
        ax = axes[idx]
        ax.set_title(f'{test} Throughput')
        ax.set_xlabel('Variant')
        ax.set_ylabel('Throughput (ops/sec)')
        
        # Collect data for this test across platforms
        test_data = [d for d in data if d.test == test]
        if not test_data:
            continue
        
        variants = sorted(set(d.variant for d in test_data))
        x_pos = range(len(variants))
        
        for platform in platforms:
            platform_data = [d for d in test_data if d.platform == platform]
            throughputs = []
            for variant in variants:
                variant_data = [d for d in platform_data if d.variant == variant]
                avg_throughput = sum(d.throughput for d in variant_data) / len(variant_data) if variant_data else 0
                throughputs.append(avg_throughput)
            
            ax.plot(x_pos, throughputs, marker='o', label=platform, linewidth=2)
        
        ax.set_xticks(x_pos)
        ax.set_xticklabels(variants, rotation=45, ha='right')
        ax.legend()
        ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    chart_path = output_dir / 'performance-comparison.png'
    plt.savefig(chart_path, dpi=150, bbox_inches='tight')
    print(f"Saved chart: {chart_path}")
    plt.close()
    
    # Chart 2: Speedup over native
    fig, ax = plt.subplots(figsize=(12, 6))
    ax.set_title('Speedup vs Native Implementation')
    ax.set_xlabel('Test Configuration')
    ax.set_ylabel('Speedup (x times faster)')
    ax.axhline(y=1.0, color='red', linestyle='--', label='Native baseline')
    
    speedups = []
    labels = []
    
    for key in sorted(grouped.keys()):
        platform, test, threads, param = key
        results = grouped[key]
        
        native = next((r for r in results if r.variant == 'native'), None)
        if not native:
            continue
        
        for result in results:
            if result.variant != 'native':
                speedup = calculate_speedup(native.throughput, result.throughput)
                speedups.append(speedup)
                labels.append(f"{platform}\n{test}\n{result.variant}")
    
    x_pos = range(len(speedups))
    colors = ['green' if s > 1.0 else 'orange' for s in speedups]
    ax.bar(x_pos, speedups, color=colors, alpha=0.7)
    ax.set_xticks(x_pos)
    ax.set_xticklabels(labels, rotation=45, ha='right', fontsize=8)
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    speedup_chart = output_dir / 'speedup-comparison.png'
    plt.savefig(speedup_chart, dpi=150, bbox_inches='tight')
    print(f"Saved chart: {speedup_chart}")
    plt.close()


def generate_json_summary(data: List[PerformanceData], output_file: Path):
    """Generate JSON summary for programmatic access"""
    summary = {
        'metadata': {
            'total_benchmarks': len(data),
            'platforms': sorted(set(d.platform for d in data)),
            'tests': sorted(set(d.test for d in data)),
            'variants': sorted(set(d.variant for d in data))
        },
        'results': []
    }
    
    grouped = group_by_configuration(data)
    
    for key in sorted(grouped.keys()):
        platform, test, threads, param = key
        results = grouped[key]
        
        native = next((r for r in results if r.variant == 'native'), None)
        native_throughput = native.throughput if native else 0.0
        
        config = {
            'platform': platform,
            'test': test,
            'threads': threads,
            'param': param,
            'variants': []
        }
        
        for result in sorted(results, key=lambda x: x.variant):
            config['variants'].append({
                'variant': result.variant,
                'throughput': result.throughput,
                'elapsed_sec': result.elapsed_sec,
                'speedup': calculate_speedup(native_throughput, result.throughput)
            })
        
        summary['results'].append(config)
    
    with open(output_file, 'w') as f:
        json.dump(summary, f, indent=2)
    
    print(f"Saved JSON summary: {output_file}")


def main():
    parser = argparse.ArgumentParser(description='Analyze fastcond performance data')
    parser.add_argument('csv_files', nargs='+', type=Path,
                        help='CSV files containing performance data')
    parser.add_argument('--output-dir', type=Path, default=Path('docs'),
                        help='Output directory for generated files (default: docs)')
    parser.add_argument('--no-charts', action='store_true',
                        help='Skip chart generation')
    
    args = parser.parse_args()
    
    # Create output directory
    args.output_dir.mkdir(parents=True, exist_ok=True)
    
    # Load data
    data = load_performance_data(args.csv_files)
    if not data:
        print("Error: No performance data loaded")
        return 1
    
    # Generate comparison table
    table = generate_comparison_table(data)
    table_file = args.output_dir / 'performance-comparison.md'
    with open(table_file, 'w') as f:
        f.write(table)
    print(f"Saved comparison table: {table_file}")
    
    # Generate JSON summary
    json_file = args.output_dir / 'performance-summary.json'
    generate_json_summary(data, json_file)
    
    # Generate charts
    if not args.no_charts:
        generate_charts(data, args.output_dir)
    
    print("\n✅ Performance analysis complete!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
