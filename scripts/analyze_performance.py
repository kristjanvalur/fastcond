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

    matplotlib.use("Agg")  # Non-interactive backend
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("Warning: matplotlib not available, charts will not be generated")


class PerformanceData:
    """Container for performance benchmark results"""

    def __init__(
        self,
        platform: str,
        os_version: str,
        test: str,
        variant: str,
        threads: int,
        param: int,
        iterations: int,
        elapsed_sec: float,
        throughput: float,
    ):
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
    def from_csv_row(cls, row: Dict[str, str]) -> "PerformanceData":
        """Parse CSV row into PerformanceData object"""
        return cls(
            platform=row["platform"],
            os_version=row["os_version"],
            test=row["test"],
            variant=row["variant"],
            threads=int(row["threads"]),
            param=int(row["param"]),
            iterations=int(row["iterations"]),
            elapsed_sec=float(row["elapsed_sec"]),
            throughput=float(row["throughput"]),
        )

    def key(self) -> Tuple:
        """Generate unique key for grouping results"""
        return (self.platform, self.test, self.threads, self.param)

    def __repr__(self) -> str:
        return (
            f"PerformanceData({self.platform}, {self.test}, {self.variant}, "
            f"threads={self.threads}, throughput={self.throughput:.2f})"
        )


def load_performance_data(csv_files: List[Path]) -> List[PerformanceData]:
    """Load and merge performance data from multiple CSV files"""
    results = []

    for csv_file in csv_files:
        if not csv_file.exists():
            print(f"Warning: File not found: {csv_file}")
            continue

        print(f"Loading data from {csv_file}...")
        with open(csv_file, "r") as f:
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


def group_by_configuration(
    data: List[PerformanceData],
) -> Dict[Tuple, List[PerformanceData]]:
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
    table.append(
        "| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |"
    )
    table.append(
        "|----------|------|---------|---------|---------------------|-------------------|"
    )

    for key in sorted(grouped.keys()):
        platform, test, threads, param = key
        results = grouped[key]

        # Find native baseline
        native = next((r for r in results if r.variant == "native"), None)
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
    """Generate per-platform comparative performance charts"""
    if not HAS_MATPLOTLIB:
        print("Skipping chart generation (matplotlib not available)")
        return

    grouped = group_by_configuration(data)
    platforms = sorted(set(d.platform for d in data))
    tests = ["qtest", "strongtest", "gil_test"]
    
    # Chart 1: Per-platform speedup comparison (most important)
    num_platforms = len(platforms)
    fig, axes = plt.subplots(1, num_platforms, figsize=(6 * num_platforms, 6))
    if num_platforms == 1:
        axes = [axes]  # Make it iterable
    
    fig.suptitle("fastcond Speedup vs Native (per platform)", fontsize=16, fontweight='bold')
    
    for idx, platform in enumerate(platforms):
        ax = axes[idx]
        ax.set_title(f"{platform.capitalize()}", fontsize=14)
        ax.set_xlabel("Test", fontsize=12)
        ax.set_ylabel("Speedup (×)", fontsize=12)
        ax.axhline(y=1.0, color="red", linestyle="--", linewidth=2, label="Native baseline", alpha=0.7)
        
        # Collect speedups for this platform
        speedup_data = {}  # {test: {variant: speedup}}
        
        for key in grouped.keys():
            plat, test, threads, param = key
            if plat != platform:
                continue
                
            results = grouped[key]
            native = next((r for r in results if "native" in r.variant), None)
            if not native:
                continue
            
            if test not in speedup_data:
                speedup_data[test] = {}
            
            for result in results:
                if "native" not in result.variant:  # Skip native, only show fastcond variants
                    speedup = calculate_speedup(native.throughput, result.throughput)
                    # Use simpler variant names for chart
                    variant_name = result.variant.replace("fastcond_", "").replace("_gil", "")
                    speedup_data[test][variant_name] = speedup
        
        # Plot grouped bars per test
        if speedup_data:
            test_names = sorted(speedup_data.keys())
            variants = sorted(set(v for test_variants in speedup_data.values() for v in test_variants.keys()))
            
            x = range(len(test_names))
            width = 0.8 / max(len(variants), 1)
            
            for i, variant in enumerate(variants):
                speedups = [speedup_data[test].get(variant, 0) for test in test_names]
                offset = (i - len(variants) / 2) * width + width / 2
                colors = ["green" if s >= 1.0 else "orange" for s in speedups]
                ax.bar([xi + offset for xi in x], speedups, width, 
                      label=variant, color=colors, alpha=0.7, edgecolor='black')
            
            ax.set_xticks(x)
            ax.set_xticklabels(test_names, fontsize=10)
            ax.legend(fontsize=10)
            ax.grid(True, alpha=0.3, axis="y")
            ax.set_ylim(bottom=0)
    
    plt.tight_layout()
    speedup_chart = output_dir / "speedup-comparison.png"
    plt.savefig(speedup_chart, dpi=150, bbox_inches="tight")
    print(f"Saved chart: {speedup_chart}")
    plt.close()
    
    # Chart 2: Per-platform throughput comparison (secondary, for absolute numbers)
    fig, axes = plt.subplots(1, num_platforms, figsize=(6 * num_platforms, 6))
    if num_platforms == 1:
        axes = [axes]
    
    fig.suptitle("Throughput by Variant (per platform)", fontsize=16, fontweight='bold')
    
    for idx, platform in enumerate(platforms):
        ax = axes[idx]
        ax.set_title(f"{platform.capitalize()}", fontsize=14)
        ax.set_xlabel("Test", fontsize=12)
        ax.set_ylabel("Throughput (ops/sec)", fontsize=12)
        
        # Collect throughput for this platform
        throughput_data = {}  # {test: {variant: throughput}}
        
        for key in grouped.keys():
            plat, test, threads, param = key
            if plat != platform:
                continue
                
            results = grouped[key]
            if test not in throughput_data:
                throughput_data[test] = {}
            
            for result in results:
                # Average multiple runs
                variant_name = result.variant.replace("fastcond_", "").replace("_gil", "")
                if variant_name in throughput_data[test]:
                    throughput_data[test][variant_name].append(result.throughput)
                else:
                    throughput_data[test][variant_name] = [result.throughput]
        
        # Average the throughputs
        for test in throughput_data:
            for variant in throughput_data[test]:
                throughput_data[test][variant] = sum(throughput_data[test][variant]) / len(throughput_data[test][variant])
        
        # Plot grouped bars per test
        if throughput_data:
            test_names = sorted(throughput_data.keys())
            variants = sorted(set(v for test_variants in throughput_data.values() for v in test_variants.keys()))
            
            x = range(len(test_names))
            width = 0.8 / max(len(variants), 1)
            
            for i, variant in enumerate(variants):
                throughputs = [throughput_data[test].get(variant, 0) for test in test_names]
                offset = (i - len(variants) / 2) * width + width / 2
                ax.bar([xi + offset for xi in x], throughputs, width, 
                      label=variant, alpha=0.7, edgecolor='black')
            
            ax.set_xticks(x)
            ax.set_xticklabels(test_names, fontsize=10)
            ax.legend(fontsize=10)
            ax.grid(True, alpha=0.3, axis="y")
            ax.set_ylim(bottom=0)
            # Use scientific notation for large numbers
            ax.ticklabel_format(style='scientific', axis='y', scilimits=(0,0))
    
    plt.tight_layout()
    throughput_chart = output_dir / "performance-comparison.png"
    plt.savefig(throughput_chart, dpi=150, bbox_inches="tight")
    print(f"Saved chart: {throughput_chart}")
    plt.close()


def generate_json_summary(data: List[PerformanceData], output_file: Path):
    """Generate JSON summary for programmatic access"""
    summary = {
        "metadata": {
            "total_benchmarks": len(data),
            "platforms": sorted(set(d.platform for d in data)),
            "tests": sorted(set(d.test for d in data)),
            "variants": sorted(set(d.variant for d in data)),
        },
        "results": [],
    }

    grouped = group_by_configuration(data)

    for key in sorted(grouped.keys()):
        platform, test, threads, param = key
        results = grouped[key]

        native = next((r for r in results if r.variant == "native"), None)
        native_throughput = native.throughput if native else 0.0

        config = {
            "platform": platform,
            "test": test,
            "threads": threads,
            "param": param,
            "variants": [],
        }

        for result in sorted(results, key=lambda x: x.variant):
            config["variants"].append(
                {
                    "variant": result.variant,
                    "throughput": result.throughput,
                    "elapsed_sec": result.elapsed_sec,
                    "speedup": calculate_speedup(native_throughput, result.throughput),
                }
            )

        summary["results"].append(config)

    with open(output_file, "w") as f:
        json.dump(summary, f, indent=2)

    print(f"Saved JSON summary: {output_file}")


def main():
    parser = argparse.ArgumentParser(description="Analyze fastcond performance data")
    parser.add_argument(
        "csv_files", nargs="+", type=Path, help="CSV files containing performance data"
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("docs"),
        help="Output directory for generated files (default: docs)",
    )
    parser.add_argument(
        "--no-charts", action="store_true", help="Skip chart generation"
    )

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
    table_file = args.output_dir / "performance-comparison.md"
    with open(table_file, "w") as f:
        f.write(table)
    print(f"Saved comparison table: {table_file}")

    # Generate JSON summary
    json_file = args.output_dir / "performance-summary.json"
    generate_json_summary(data, json_file)

    # Generate charts
    if not args.no_charts:
        generate_charts(data, args.output_dir)

    print("\n✅ Performance analysis complete!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
