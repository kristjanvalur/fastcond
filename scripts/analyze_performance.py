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
    """Generate per-test, per-platform comparative performance charts"""
    if not HAS_MATPLOTLIB:
        print("Skipping chart generation (matplotlib not available)")
        return

    grouped = group_by_configuration(data)
    platforms = sorted(set(d.platform for d in data))
    tests = ["qtest", "strongtest", "gil_test"]

    # Generate separate charts for each test type
    for test_name in tests:
        test_data = {k: v for k, v in grouped.items() if k[1] == test_name}
        if not test_data:
            print(f"No data for {test_name}, skipping charts")
            continue

        # Chart 1: Per-platform speedup comparison (most important)
        num_platforms = len(platforms)
        fig, axes = plt.subplots(1, num_platforms, figsize=(6 * num_platforms, 5))
        if num_platforms == 1:
            axes = [axes]  # Make it iterable

        fig.suptitle(f"{test_name} - Speedup vs Native", fontsize=16, fontweight="bold")
        fig.suptitle(f"{test_name} - Speedup vs Native", fontsize=16, fontweight="bold")

        for idx, platform in enumerate(platforms):
            ax = axes[idx]
            ax.set_title(f"{platform.capitalize()}", fontsize=14)
            ax.set_ylabel("Speedup (×)", fontsize=12)
            ax.axhline(
                y=1.0,
                color="red",
                linestyle="--",
                linewidth=2,
                label="Native baseline",
                alpha=0.7,
            )

            # Collect speedups for this platform and test
            speedup_by_variant = {}  # {variant: [speedups]}

            for key in test_data.keys():
                plat, test, threads, param = key
                if plat != platform:
                    continue

                results = test_data[key]
                native = next((r for r in results if "native" in r.variant), None)
                if not native:
                    continue

                for result in results:
                    if (
                        "native" not in result.variant
                    ):  # Skip native, only show fastcond variants
                        speedup = calculate_speedup(
                            native.throughput, result.throughput
                        )
                        # Use simpler variant names for chart
                        variant_name = result.variant.replace("fastcond_", "").replace(
                            "_gil", ""
                        )
                        if variant_name not in speedup_by_variant:
                            speedup_by_variant[variant_name] = []
                        speedup_by_variant[variant_name].append(speedup)

            # Plot bars for each variant (average if multiple data points)
            if speedup_by_variant:
                variants = sorted(speedup_by_variant.keys())
                avg_speedups = [
                    sum(speedup_by_variant[v]) / len(speedup_by_variant[v])
                    for v in variants
                ]

                colors = ["green" if s >= 1.0 else "orange" for s in avg_speedups]
                bars = ax.bar(
                    variants, avg_speedups, color=colors, alpha=0.7, edgecolor="black"
                )

                # Add value labels on bars
                for bar, speedup in zip(bars, avg_speedups):
                    height = bar.get_height()
                    ax.text(
                        bar.get_x() + bar.get_width() / 2.0,
                        height,
                        f"{speedup:.2f}×",
                        ha="center",
                        va="bottom",
                        fontsize=10,
                        fontweight="bold",
                    )

                ax.grid(True, alpha=0.3, axis="y")
                ax.set_ylim(bottom=0, top=max(avg_speedups) * 1.2)

        plt.tight_layout()
        speedup_chart = output_dir / f"{test_name}-speedup.png"
        plt.savefig(speedup_chart, dpi=150, bbox_inches="tight")
        print(f"Saved chart: {speedup_chart}")
        plt.close()

        # Chart 2: Per-platform throughput comparison (secondary, for absolute numbers)
        fig, axes = plt.subplots(1, num_platforms, figsize=(6 * num_platforms, 5))
        if num_platforms == 1:
            axes = [axes]

        fig.suptitle(
            f"{test_name} - Throughput by Variant", fontsize=16, fontweight="bold"
        )

        for idx, platform in enumerate(platforms):
            ax = axes[idx]
            ax.set_title(f"{platform.capitalize()}", fontsize=14)
            ax.set_ylabel("Throughput (ops/sec)", fontsize=12)

            # Collect throughput for this platform and test
            throughput_by_variant = {}  # {variant: [throughputs]}

            for key in test_data.keys():
                plat, test, threads, param = key
                if plat != platform:
                    continue

                results = test_data[key]
                for result in results:
                    # Use simpler variant names for chart
                    # For GIL tests, use "fastcond" vs "native" labels
                    if "fastcond" in result.variant:
                        variant_name = "fastcond"
                    elif "native" in result.variant:
                        variant_name = "native"
                    else:
                        # For other tests, clean up variant names
                        variant_name = result.variant.replace("fastcond_", "").replace(
                            "_gil", ""
                        )

                    if variant_name not in throughput_by_variant:
                        throughput_by_variant[variant_name] = []
                    throughput_by_variant[variant_name].append(result.throughput)

            # Plot bars for each variant (average if multiple data points)
            if throughput_by_variant:
                variants = sorted(throughput_by_variant.keys())
                avg_throughputs = [
                    sum(throughput_by_variant[v]) / len(throughput_by_variant[v])
                    for v in variants
                ]

                bars = ax.bar(variants, avg_throughputs, alpha=0.7, edgecolor="black")

                # Add value labels on bars
                for bar, throughput in zip(bars, avg_throughputs):
                    height = bar.get_height()
                    # Format large numbers with K/M suffix
                    if throughput >= 1e6:
                        label = f"{throughput / 1e6:.1f}M"
                    elif throughput >= 1e3:
                        label = f"{throughput / 1e3:.0f}K"
                    else:
                        label = f"{throughput:.0f}"
                    ax.text(
                        bar.get_x() + bar.get_width() / 2.0,
                        height,
                        label,
                        ha="center",
                        va="bottom",
                        fontsize=10,
                        fontweight="bold",
                    )

                ax.grid(True, alpha=0.3, axis="y")
                ax.set_ylim(bottom=0, top=max(avg_throughputs) * 1.2)
                # Use scientific notation for y-axis if numbers are large
                if max(avg_throughputs) >= 10000:
                    ax.ticklabel_format(style="scientific", axis="y", scilimits=(0, 0))

        plt.tight_layout()
        throughput_chart = output_dir / f"{test_name}-throughput.png"
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
