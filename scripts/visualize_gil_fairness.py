#!/usr/bin/env python3
"""
Visualization script for GIL fairness comparison.

Creates specialized charts highlighting the differences between fair and unfair
GIL implementations, focusing on fairness metrics rather than backend differences.
"""

import json
import sys
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import argparse


def load_benchmark_data(json_file):
    """Load and filter GIL test results from benchmark JSON."""
    with open(json_file, "r") as f:
        data = json.load(f)

    # Filter for GIL tests only
    gil_results = [result for result in data if result.get("benchmark") == "gil_test"]

    if not gil_results:
        raise ValueError("No GIL test results found in benchmark data")

    return gil_results


def extract_fairness_metrics(results):
    """Extract fairness metrics for comparison."""
    fair_data = None
    unfair_data = None

    for result in results:
        fairness_mode = result.get("fairness_mode", "unknown")
        fairness_stats = result.get("results", {}).get("fairness_stats", {})
        overall_stats = result.get("results", {}).get("overall", {})

        metrics = {
            "coefficient_of_variation": fairness_stats.get(
                "coefficient_of_variation", 0
            ),
            "transition_percentage": fairness_stats.get("transition_percentage", 0),
            "consecutive_percentage": fairness_stats.get("consecutive_percentage", 0),
            "fairness_score": fairness_stats.get("fairness_score", 0),
            "operations_per_sec": overall_stats.get("operations_per_sec", 0),
            "avg_latency_us": overall_stats.get("avg_latency_us", 0),
            "thread_acquisitions": fairness_stats.get("thread_acquisitions", []),
        }

        if fairness_mode == "fair":
            fair_data = metrics
        elif fairness_mode == "unfair":
            unfair_data = metrics

    return fair_data, unfair_data


def create_fairness_comparison_chart(fair_data, unfair_data, output_dir):
    """Create a comprehensive fairness comparison chart."""

    # Set up the figure with multiple subplots
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
    fig.suptitle(
        "GIL Fairness Analysis: Fair vs Unfair Implementation",
        fontsize=16,
        fontweight="bold",
    )

    # 1. Thread acquisition distribution
    if fair_data["thread_acquisitions"] and unfair_data["thread_acquisitions"]:
        fair_acq = [t["acquisitions"] for t in fair_data["thread_acquisitions"]]
        unfair_acq = [t["acquisitions"] for t in unfair_data["thread_acquisitions"]]

        thread_ids = range(len(fair_acq))
        x = np.arange(len(thread_ids))
        width = 0.35

        ax1.bar(
            x - width / 2, fair_acq, width, label="Fair GIL", color="skyblue", alpha=0.8
        )
        ax1.bar(
            x + width / 2,
            unfair_acq,
            width,
            label="Unfair GIL",
            color="lightcoral",
            alpha=0.8,
        )

        ax1.set_xlabel("Thread ID")
        ax1.set_ylabel("Acquisitions")
        ax1.set_title("Thread Acquisition Distribution")
        ax1.set_xticks(x)
        ax1.set_xticklabels([f"Thread {i}" for i in thread_ids])
        ax1.legend()
        ax1.grid(True, alpha=0.3)

    # 2. Fairness metrics comparison
    metrics = [
        "Coefficient of\nVariation",
        "Consecutive\nRe-acquisitions (%)",
        "Fairness\nScore",
    ]
    fair_values = [
        fair_data["coefficient_of_variation"],
        fair_data["consecutive_percentage"],
        fair_data["fairness_score"],
    ]
    unfair_values = [
        unfair_data["coefficient_of_variation"],
        unfair_data["consecutive_percentage"],
        unfair_data["fairness_score"],
    ]

    x = np.arange(len(metrics))
    width = 0.35

    ax2.bar(
        x - width / 2, fair_values, width, label="Fair GIL", color="skyblue", alpha=0.8
    )
    ax2.bar(
        x + width / 2,
        unfair_values,
        width,
        label="Unfair GIL",
        color="lightcoral",
        alpha=0.8,
    )

    ax2.set_xlabel("Metric")
    ax2.set_ylabel("Value (lower is better)")
    ax2.set_title("Fairness Metrics Comparison")
    ax2.set_xticks(x)
    ax2.set_xticklabels(metrics, rotation=0, ha="center")
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    # Add value labels on bars
    for i, (fair_val, unfair_val) in enumerate(zip(fair_values, unfair_values)):
        ax2.text(
            i - width / 2,
            fair_val + max(fair_values + unfair_values) * 0.01,
            f"{fair_val:.3f}",
            ha="center",
            va="bottom",
            fontsize=9,
        )
        ax2.text(
            i + width / 2,
            unfair_val + max(fair_values + unfair_values) * 0.01,
            f"{unfair_val:.3f}",
            ha="center",
            va="bottom",
            fontsize=9,
        )

    # 3. Performance comparison
    perf_metrics = ["GIL Cycles/sec", "Avg Latency (μs)"]

    # Normalize to percentages for comparison (with safety checks)
    latency_norm = (
        (fair_data["avg_latency_us"] / unfair_data["avg_latency_us"]) * 100
        if unfair_data["avg_latency_us"] > 0
        else 100
    )
    ops_norm = (
        (unfair_data["operations_per_sec"] / fair_data["operations_per_sec"]) * 100
        if fair_data["operations_per_sec"] > 0
        else 100
    )

    fair_perf_norm = [100, latency_norm]
    unfair_perf_norm = [ops_norm, 100]

    x = np.arange(len(perf_metrics))

    ax3.bar(
        x - width / 2,
        fair_perf_norm,
        width,
        label="Fair GIL",
        color="skyblue",
        alpha=0.8,
    )
    ax3.bar(
        x + width / 2,
        unfair_perf_norm,
        width,
        label="Unfair GIL",
        color="lightcoral",
        alpha=0.8,
    )

    ax3.set_xlabel("Metric")
    ax3.set_ylabel("Relative Performance (%)")
    ax3.set_title("Performance Comparison (Normalized)")
    ax3.set_xticks(x)
    ax3.set_xticklabels(perf_metrics)
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.axhline(y=100, color="black", linestyle="--", alpha=0.5)

    # Add actual values as labels
    ax3.text(
        0 - width / 2,
        fair_perf_norm[0] + 5,
        f"{fair_data['operations_per_sec']:,}",
        ha="center",
        va="bottom",
        fontsize=9,
        rotation=90,
    )
    ax3.text(
        0 + width / 2,
        unfair_perf_norm[0] + 5,
        f"{unfair_data['operations_per_sec']:,}",
        ha="center",
        va="bottom",
        fontsize=9,
        rotation=90,
    )
    ax3.text(
        1 - width / 2,
        fair_perf_norm[1] + 5,
        f"{fair_data['avg_latency_us']:.1f}μs",
        ha="center",
        va="bottom",
        fontsize=9,
        rotation=90,
    )
    ax3.text(
        1 + width / 2,
        unfair_perf_norm[1] + 5,
        f"{unfair_data['avg_latency_us']:.1f}μs",
        ha="center",
        va="bottom",
        fontsize=9,
        rotation=90,
    )

    # 4. Summary text
    ax4.axis("off")

    summary_text = f"""
    Fair GIL Analysis Summary
    
    Fairness Metrics:
    • Coefficient of Variation: {fair_data["coefficient_of_variation"]:.4f} (fair) vs {unfair_data["coefficient_of_variation"]:.4f} (unfair)
    • Thread Transitions: {fair_data["transition_percentage"]:.1f}% (fair) vs {unfair_data["transition_percentage"]:.1f}% (unfair)
    • Consecutive Re-acquisitions: {fair_data["consecutive_percentage"]:.1f}% (fair) vs {unfair_data["consecutive_percentage"]:.1f}% (unfair)
    • Fairness Score: {fair_data["fairness_score"]:.3f} (fair) vs {unfair_data["fairness_score"]:.3f} (unfair)
    
    Performance Impact:
    • GIL Acquire/Release Cycles/sec: {fair_data["operations_per_sec"]:,} (fair) vs {unfair_data["operations_per_sec"]:,} (unfair)
    • Latency: {fair_data["avg_latency_us"]:.1f}μs (fair) vs {unfair_data["avg_latency_us"]:.1f}μs (unfair)
    
    Key Insight:
    Fair GIL prevents thread starvation at minimal performance cost.
    Lower fairness scores indicate more equitable thread scheduling.
    """

    ax4.text(
        0.05,
        0.95,
        summary_text,
        transform=ax4.transAxes,
        fontsize=11,
        verticalalignment="top",
        bbox=dict(boxstyle="round", facecolor="lightgray", alpha=0.8),
    )

    plt.tight_layout()

    # Save the chart
    output_path = Path(output_dir) / "gil_fairness_comparison.png"
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    print(f"Fairness comparison chart saved to {output_path}")

    return output_path


def main():
    """Generate GIL fairness visualization from benchmark data."""
    parser = argparse.ArgumentParser(
        description="Generate GIL fairness comparison charts"
    )
    parser.add_argument("benchmark_json", help="Path to benchmark JSON file")
    parser.add_argument(
        "--output-dir", default="./charts", help="Output directory for charts"
    )

    args = parser.parse_args()

    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)

    try:
        # Load and process data
        results = load_benchmark_data(args.benchmark_json)
        fair_data, unfair_data = extract_fairness_metrics(results)

        if not fair_data or not unfair_data:
            print(
                "Error: Could not find both fair and unfair GIL test results",
                file=sys.stderr,
            )
            sys.exit(1)

        # Generate visualization
        chart_path = create_fairness_comparison_chart(
            fair_data, unfair_data, output_dir
        )

        print("GIL fairness analysis complete!")
        print(f"Chart generated: {chart_path}")

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
