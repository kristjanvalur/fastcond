#!/usr/bin/env python3
"""
Analyze benchmark trends from historical data.

This script analyzes historical benchmark results to show performance
trends over time, including statistical significance of differences.
"""

import argparse
import json
import statistics
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path
from typing import Dict, Any

try:
    import matplotlib.pyplot as plt
    import matplotlib.dates as mdates
    import matplotlib

    matplotlib.use("Agg")
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("⚠️  matplotlib not available, charts will not be generated", file=sys.stderr)


def load_history(history_file: Path) -> Dict[str, Any]:
    """Load historical benchmark data."""
    with open(history_file) as f:
        return json.load(f)


def calculate_speedup(native_throughput: float, fastcond_throughput: float) -> float:
    """Calculate speedup ratio."""
    if native_throughput == 0:
        return 0.0
    return fastcond_throughput / native_throughput


def analyze_trends(history: Dict[str, Any]) -> Dict[str, Any]:
    """Analyze benchmark trends across historical runs."""
    runs = history["runs"]

    if not runs:
        return {"error": "No historical data available"}

    # Group results by (benchmark, platform)
    trends = defaultdict(
        lambda: {"native": [], "fastcond": [], "speedup": [], "timestamps": []}
    )

    for run in runs:
        timestamp = datetime.fromisoformat(run["timestamp"].replace("Z", "+00:00"))

        for result in run["results"]:
            benchmark = result["benchmark"]
            implementation = result["implementation"]
            platform = result.get("system", {}).get("os", "unknown")
            throughput = result["results"]["overall"]["throughput_items_per_sec"]

            key = f"{benchmark}_{platform}"

            if "native" in implementation:
                trends[key]["native"].append((timestamp, throughput))
            elif "fastcond" in implementation:
                trends[key]["fastcond"].append((timestamp, throughput))

    # Calculate speedup trends
    for key, data in trends.items():
        # Match up native and fastcond runs by timestamp
        native_dict = {ts: tp for ts, tp in data["native"]}
        fastcond_dict = {ts: tp for ts, tp in data["fastcond"]}

        for ts in sorted(set(native_dict.keys()) & set(fastcond_dict.keys())):
            native_tp = native_dict[ts]
            fastcond_tp = fastcond_dict[ts]
            speedup = calculate_speedup(native_tp, fastcond_tp)
            trends[key]["speedup"].append((ts, speedup))
            trends[key]["timestamps"].append(ts)

    return dict(trends)


def generate_trend_report(trends: Dict[str, Any], output_file: Path) -> None:
    """Generate markdown report of trends."""
    lines = ["# Benchmark Performance Trends", ""]
    lines.append(
        f"*Analysis of {sum(len(v['speedup']) for v in trends.values())} historical data points*"
    )
    lines.append("")

    for key, data in sorted(trends.items()):
        if not data["speedup"]:
            continue

        lines.append(f"## {key.replace('_', ' - ')}")
        lines.append("")

        speedups = [s for _, s in data["speedup"]]
        timestamps = data["timestamps"]

        if len(speedups) >= 2:
            mean_speedup = statistics.mean(speedups)
            stdev_speedup = statistics.stdev(speedups)
            min_speedup = min(speedups)
            max_speedup = max(speedups)

            # Calculate trend (simple linear regression slope)
            n = len(speedups)
            x = list(range(n))
            y = speedups
            x_mean = statistics.mean(x)
            y_mean = mean_speedup

            slope = sum((x[i] - x_mean) * (y[i] - y_mean) for i in range(n)) / sum(
                (x[i] - x_mean) ** 2 for i in range(n)
            )

            trend_direction = (
                "📈 Improving"
                if slope > 0.001
                else "📉 Declining"
                if slope < -0.001
                else "➡️  Stable"
            )

            lines.append(
                f"**Data Points:** {n} runs from {timestamps[0].date()} to {timestamps[-1].date()}"
            )
            lines.append("")
            lines.append("| Metric | Value |")
            lines.append("|--------|-------|")
            lines.append(
                f"| Mean Speedup | **{mean_speedup:.3f}×** ({(mean_speedup - 1) * 100:+.1f}%) |"
            )
            lines.append(f"| Std Deviation | {stdev_speedup:.3f}× |")
            lines.append(f"| Range | {min_speedup:.3f}× to {max_speedup:.3f}× |")
            lines.append(f"| Trend | {trend_direction} (slope: {slope:.4f}) |")
            lines.append("")

            # Statistical significance
            if mean_speedup > 1.0:
                # Calculate t-statistic
                import math

                t_stat = (mean_speedup - 1.0) / (stdev_speedup / math.sqrt(n))
                # Approximate p-value check (t > 2.0 is roughly p < 0.05 for most sample sizes)
                is_significant = abs(t_stat) > 2.0
                significance = (
                    "✅ Statistically significant"
                    if is_significant
                    else "⚠️  Not statistically significant"
                )
                lines.append(
                    f"**Performance Difference:** {significance} (t={t_stat:.2f})"
                )
                lines.append("")

        else:
            lines.append(
                "*Insufficient data for trend analysis (need at least 2 runs)*"
            )
            lines.append("")

    with open(output_file, "w") as f:
        f.write("\n".join(lines))

    print(f"📊 Trend report saved to {output_file}", file=sys.stderr)


def generate_trend_charts(trends: Dict[str, Any], output_dir: Path) -> None:
    """Generate trend visualization charts."""
    if not HAS_MATPLOTLIB:
        return

    for key, data in trends.items():
        if not data["speedup"]:
            continue

        timestamps = [ts for ts, _ in data["speedup"]]
        speedups = [s for _, s in data["speedup"]]

        if len(timestamps) < 2:
            continue

        # Create figure
        fig, ax = plt.subplots(figsize=(12, 6))

        # Plot speedup over time
        ax.plot(
            timestamps,
            speedups,
            marker="o",
            linestyle="-",
            linewidth=2,
            markersize=6,
            label="Speedup",
        )

        # Add mean line
        mean_speedup = statistics.mean(speedups)
        ax.axhline(
            y=mean_speedup,
            color="green",
            linestyle="--",
            linewidth=2,
            alpha=0.7,
            label=f"Mean: {mean_speedup:.3f}×",
        )

        # Add baseline
        ax.axhline(
            y=1.0,
            color="red",
            linestyle="--",
            linewidth=2,
            alpha=0.7,
            label="Baseline (1.0×)",
        )

        # Formatting
        ax.set_xlabel("Date", fontsize=12, fontweight="bold")
        ax.set_ylabel("Speedup (fastcond / native)", fontsize=12, fontweight="bold")
        ax.set_title(
            f"{key.replace('_', ' - ')} Performance Trend",
            fontsize=14,
            fontweight="bold",
        )
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=10)

        # Format x-axis dates
        ax.xaxis.set_major_formatter(mdates.DateFormatter("%Y-%m-%d"))
        ax.xaxis.set_major_locator(mdates.AutoDateLocator())
        plt.xticks(rotation=45)

        # Add confidence band (mean ± stdev)
        if len(speedups) >= 3:
            stdev = statistics.stdev(speedups)
            ax.fill_between(
                timestamps,
                [mean_speedup - stdev] * len(timestamps),
                [mean_speedup + stdev] * len(timestamps),
                alpha=0.2,
                color="green",
                label="±1 std dev",
            )

        plt.tight_layout()

        # Save chart
        chart_file = output_dir / f"{key}-trend.png"
        plt.savefig(chart_file, dpi=150, bbox_inches="tight")
        plt.close()

        print(f"📈 Chart saved: {chart_file}", file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(description="Analyze benchmark performance trends")
    parser.add_argument(
        "history_file",
        type=Path,
        help="Historical benchmark data file (benchmark-history.json)",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("trend-analysis"),
        help="Output directory for analysis results (default: trend-analysis)",
    )
    parser.add_argument(
        "--no-charts", action="store_true", help="Skip chart generation"
    )

    args = parser.parse_args()

    if not args.history_file.exists():
        print(f"❌ History file not found: {args.history_file}", file=sys.stderr)
        return 1

    # Create output directory
    args.output_dir.mkdir(parents=True, exist_ok=True)

    # Load historical data
    print(f"📥 Loading historical data from {args.history_file}", file=sys.stderr)
    history = load_history(args.history_file)

    print(f"✅ Loaded {len(history.get('runs', []))} historical runs", file=sys.stderr)

    if not history.get("runs"):
        print("❌ No historical data available for analysis", file=sys.stderr)
        return 1

    # Analyze trends
    print("🔍 Analyzing performance trends...", file=sys.stderr)
    trends = analyze_trends(history)

    # Generate report
    report_file = args.output_dir / "trend-analysis.md"
    generate_trend_report(trends, report_file)

    # Generate charts
    if not args.no_charts:
        print("📊 Generating trend charts...", file=sys.stderr)
        generate_trend_charts(trends, args.output_dir)

    print(
        f"\n✅ Trend analysis complete! Results in {args.output_dir}", file=sys.stderr
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
