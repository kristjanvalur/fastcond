#!/usr/bin/env python3
"""
Robust benchmark runner with statistical analysis.

Runs benchmarks multiple times with proper warm-up and statistical reporting.
Uses JSON output from tests for robust parsing.
"""

import subprocess
import json
import sys
import os
import statistics
import time
from dataclasses import dataclass
from typing import List, Dict, Any, Optional
from pathlib import Path


@dataclass
class BenchmarkRun:
    """Single benchmark run results."""

    throughput: float
    total_time: float
    latency_avg: Optional[float] = None
    latency_stdev: Optional[float] = None
    latency_min: Optional[float] = None
    latency_max: Optional[float] = None
    # Per-thread data for backward compatibility with visualize.py
    per_thread: Optional[List[Dict[str, Any]]] = None
    # Extensible for future metrics
    false_wakeups: Optional[int] = None
    spurious_wakeups: Optional[int] = None
    custom_metrics: Optional[Dict[str, Any]] = None


@dataclass
class BenchmarkStatistics:
    """Statistical summary of multiple runs."""

    mean_throughput: float
    stdev_throughput: float
    cv_percent: float
    min_throughput: float
    max_throughput: float
    individual_runs: List[float]
    confidence_interval_95: tuple  # (lower, upper)

    # Latency statistics (if available)
    mean_latency: Optional[float] = None
    stdev_latency: Optional[float] = None

    # Spurious wakeup metrics
    total_spurious_wakeups: Optional[int] = None
    mean_spurious_wakeups: Optional[float] = None


def parse_test_output(output: str, test_type: str) -> BenchmarkRun:
    """
    Parse test output from JSON format.

    Tests output JSON when FASTCOND_JSON_OUTPUT=1 is set.
    """
    try:
        # Parse JSON output from test
        data = json.loads(output.strip())

        metrics = {
            "throughput": data["timing"]["throughput"],
            "total_time": data["timing"]["elapsed_sec"],
        }

        # Extract per-thread data if present
        per_thread_data = []
        if "per_thread" in data:
            for thread in data["per_thread"]:
                thread_info = {
                    "thread_id": thread["thread"],
                    "thread_type": thread["type"],
                    "items": thread["n_got"],
                    "spurious_wakeups": thread["spurious_wakeups"],
                }
                # Add latency stats if present
                if "latency_avg" in thread:
                    thread_info.update(
                        {
                            "avg_latency_sec": thread["latency_avg"],
                            "stdev_latency_sec": thread["latency_stdev"],
                            "min_latency_sec": thread["latency_min"],
                            "max_latency_sec": thread["latency_max"],
                        }
                    )
                per_thread_data.append(thread_info)

            # Use first receiver's latency as representative
            if per_thread_data and "avg_latency_sec" in per_thread_data[0]:
                first = per_thread_data[0]
                metrics["latency_avg"] = first["avg_latency_sec"]
                metrics["latency_stdev"] = first["stdev_latency_sec"]
                metrics["latency_min"] = first["min_latency_sec"]
                metrics["latency_max"] = first["max_latency_sec"]

            # Sum spurious wakeups across all threads
            metrics["spurious_wakeups"] = sum(
                t["spurious_wakeups"] for t in per_thread_data
            )
            metrics["per_thread"] = per_thread_data

        return BenchmarkRun(**metrics)

    except (json.JSONDecodeError, KeyError) as e:
        print(f"Error parsing JSON output: {e}", file=sys.stderr)
        print(f"Output was: {output[:200]}...", file=sys.stderr)
        raise


def run_single_benchmark(
    executable: str, args: List[str], test_type: str, timeout: int = 30
) -> Optional[BenchmarkRun]:
    """Run a single benchmark and return parsed results."""
    try:
        # Set environment to request JSON output
        env = os.environ.copy()
        env["FASTCOND_JSON_OUTPUT"] = "1"

        result = subprocess.run(
            [executable] + args,
            capture_output=True,
            text=True,
            timeout=timeout,
            env=env,
        )

        if result.returncode != 0:
            print(
                f"Warning: {executable} failed with code {result.returncode}",
                file=sys.stderr,
            )
            return None

        return parse_test_output(result.stdout, test_type)

    except subprocess.TimeoutExpired:
        print(f"Warning: {executable} timed out after {timeout}s", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Warning: {executable} failed: {e}", file=sys.stderr)
        return None


def filter_outliers_iqr(values: List[float], k: float = 1.5) -> tuple:
    """
    Filter outliers using IQR method.
    
    Args:
        values: List of numerical values
        k: IQR multiplier (1.5 = standard, 2.0 = conservative)
        
    Returns:
        Tuple of (clean_values, outliers, bounds)
    """
    if len(values) < 3:
        # Too few points for meaningful outlier detection
        return values, [], (None, None)
    
    sorted_values = sorted(values)
    n = len(sorted_values)
    
    # Calculate quartiles with proper interpolation
    def quartile(data, q):
        pos = q * (n - 1)
        if pos == int(pos):
            return data[int(pos)]
        else:
            lower = int(pos)
            upper = lower + 1
            weight = pos - lower
            return data[lower] * (1 - weight) + data[upper] * weight
    
    q1 = quartile(sorted_values, 0.25)
    q3 = quartile(sorted_values, 0.75)
    iqr = q3 - q1
    
    # Calculate outlier bounds
    lower_bound = q1 - k * iqr
    upper_bound = q3 + k * iqr
    
    # Separate inliers and outliers
    inliers = []
    outliers = []
    for x in values:
        if x < lower_bound or x > upper_bound:
            outliers.append(x)
        else:
            inliers.append(x)
    
    return inliers, outliers, (lower_bound, upper_bound)


def calculate_statistics(runs: List[BenchmarkRun]) -> BenchmarkStatistics:
    """Calculate statistical summary from multiple runs with IQR outlier filtering."""
    throughputs = [r.throughput for r in runs]
    
    # Apply IQR outlier filtering
    clean_throughputs, outliers, bounds = filter_outliers_iqr(throughputs)
    
    # Log outlier removal if any
    if outliers:
        print(f"    IQR filter removed {len(outliers)} outliers: {[f'{x:,.0f}' for x in outliers]}", file=sys.stderr)
        print(f"    IQR bounds: [{bounds[0]:,.0f}, {bounds[1]:,.0f}]", file=sys.stderr)
    
    # Use clean data for statistics
    if not clean_throughputs:
        # All runs were outliers (shouldn't happen with reasonable data)
        print("    Warning: All runs filtered as outliers, using original data", file=sys.stderr)
        clean_throughputs = throughputs
    
    mean = statistics.mean(clean_throughputs)
    stdev = statistics.stdev(clean_throughputs) if len(clean_throughputs) > 1 else 0.0
    cv = (stdev / mean * 100) if mean > 0 else 0.0

    # 95% confidence interval using t-distribution
    # For small samples, this is more accurate than normal distribution
    if len(clean_throughputs) > 1:
        import math

        # Approximate t-value for 95% CI (good enough for n >= 3)
        t_values = {
            3: 4.303,
            4: 3.182,
            5: 2.776,
            6: 2.571,
            7: 2.447,
            8: 2.365,
            10: 2.262,
        }
        t = t_values.get(len(clean_throughputs), 2.0)  # Default to ~2 for larger n
        margin = t * (stdev / math.sqrt(len(clean_throughputs)))
        ci_lower = mean - margin
        ci_upper = mean + margin
    else:
        ci_lower = ci_upper = mean

    stats = BenchmarkStatistics(
        mean_throughput=mean,
        stdev_throughput=stdev,
        cv_percent=cv,
        min_throughput=min(clean_throughputs),  # Min of CLEAN data
        max_throughput=max(clean_throughputs),  # Max of CLEAN data
        individual_runs=clean_throughputs,      # Store clean runs only
        confidence_interval_95=(ci_lower, ci_upper),
    )

    # Add latency statistics if available (also filter outliers)
    latencies = [r.latency_avg for r in runs if r.latency_avg is not None]
    if latencies:
        clean_latencies, _, _ = filter_outliers_iqr(latencies)
        if clean_latencies:
            stats.mean_latency = statistics.mean(clean_latencies)
            stats.stdev_latency = statistics.stdev(clean_latencies) if len(clean_latencies) > 1 else 0.0

    # Add spurious wakeup statistics if available
    spurious_wakeups = [
        r.spurious_wakeups for r in runs if r.spurious_wakeups is not None
    ]
    if spurious_wakeups:
        stats.total_spurious_wakeups = sum(spurious_wakeups)
        stats.mean_spurious_wakeups = statistics.mean(spurious_wakeups)

    return stats


def run_benchmark_suite(
    executable: str,
    args: List[str],
    test_type: str,
    iterations: int = 5,
    warmup: int = 1,
    timeout: int = 30,
) -> Optional[tuple]:
    """
    Run benchmark multiple times with warm-up and return statistics.

    Returns:
        Tuple of (BenchmarkStatistics, per_thread_data) or None if all runs failed

    Args:
        executable: Path to test executable
        args: Command-line arguments
        test_type: Test type (qtest, strongtest, gil_test)
        iterations: Number of measurement runs
        warmup: Number of warm-up runs (discarded)
        timeout: Timeout per run in seconds
    """
    print(
        f"Running {os.path.basename(executable)}: {warmup} warm-up + {iterations} measurement runs",
        file=sys.stderr,
    )

    # Warm-up runs
    for i in range(warmup):
        print(f"  Warm-up {i + 1}/{warmup}...", end=" ", flush=True, file=sys.stderr)
        result = run_single_benchmark(executable, args, test_type, timeout)
        if result:
            print(f"{result.throughput:,.0f} items/sec", file=sys.stderr)
        else:
            print("FAILED", file=sys.stderr)

    # Measurement runs
    runs = []
    for i in range(iterations):
        print(f"  Run {i + 1}/{iterations}...", end=" ", flush=True, file=sys.stderr)
        result = run_single_benchmark(executable, args, test_type, timeout)
        if result:
            runs.append(result)
            print(f"{result.throughput:,.0f} items/sec", file=sys.stderr)
        else:
            print("FAILED", file=sys.stderr)

    if not runs:
        print(f"Error: All runs failed for {executable}", file=sys.stderr)
        return None

    if len(runs) < iterations:
        print(f"Warning: Only {len(runs)}/{iterations} runs succeeded", file=sys.stderr)

    stats = calculate_statistics(runs)
    print(
        f"  Result: {stats.mean_throughput:,.0f} ± {stats.stdev_throughput:,.0f} items/sec (CV={stats.cv_percent:.1f}%)",
        file=sys.stderr,
    )

    # Get per_thread data from first successful run for backward compatibility
    per_thread_data = runs[0].per_thread if runs and runs[0].per_thread else None

    return (stats, per_thread_data)


def format_result_for_json(
    benchmark_name: str,
    implementation: str,
    description: str,
    stats: BenchmarkStatistics,
    config: Dict[str, Any],
    system_info: Dict[str, Any],
    per_thread_data: Optional[List[Dict[str, Any]]] = None,
) -> Dict[str, Any]:
    """Format benchmark results for JSON output (compatible with existing format)."""

    # Convert to microseconds for latency reporting
    latency_us = stats.mean_latency * 1e6 if stats.mean_latency else None
    latency_stdev_us = stats.stdev_latency * 1e6 if stats.stdev_latency else None

    result = {
        "benchmark": benchmark_name,
        "implementation": implementation,
        "description": description,
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "system": system_info,
        "config": config,
        "results": {
            "overall": {
                "throughput_items_per_sec": stats.mean_throughput,
                "throughput_stdev": stats.stdev_throughput,
                "throughput_cv_percent": stats.cv_percent,
                "throughput_ci_95": stats.confidence_interval_95,
                "total_items": config.get("total_items"),
            },
            "statistics": {
                "mean": stats.mean_throughput,
                "stdev": stats.stdev_throughput,
                "min": stats.min_throughput,
                "max": stats.max_throughput,
                "cv_percent": stats.cv_percent,
                "individual_runs": stats.individual_runs,
            },
            "latency": {
                "mean_us": latency_us,
                "stdev_us": latency_stdev_us,
            }
            if latency_us
            else None,
            "instrumentation": {
                "total_spurious_wakeups": stats.total_spurious_wakeups,
                "mean_spurious_wakeups": stats.mean_spurious_wakeups,
            }
            if stats.total_spurious_wakeups is not None
            else None,
        },
    }

    # Add per_thread data for backward compatibility with visualize.py
    if per_thread_data:
        result["results"]["per_thread"] = per_thread_data

    return result


def main():
    """Main entry point."""
    import argparse
    import platform

    parser = argparse.ArgumentParser(
        description="Run benchmarks with statistical analysis"
    )
    parser.add_argument("build_dir", help="Build directory containing test executables")
    parser.add_argument(
        "--iterations",
        "-n",
        type=int,
        default=5,
        help="Number of measurement runs per benchmark (default: 5)",
    )
    parser.add_argument(
        "--warmup",
        "-w",
        type=int,
        default=1,
        help="Number of warm-up runs (default: 1)",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=30,
        help="Timeout per run in seconds (default: 30)",
    )
    parser.add_argument(
        "--items",
        type=int,
        default=400000,
        help="Number of items for qtest/strongtest (default: 400000 for ~1s runtime)",
    )
    parser.add_argument(
        "--output-csv",
        type=str,
        metavar="FILE",
        help="Output results in CSV format compatible with run_performance_benchmarks.sh",
    )
    parser.add_argument(
        "--platform",
        type=str,
        default=None,
        help="Platform name for CSV output (default: auto-detect)",
    )
    parser.add_argument(
        "--os-version",
        type=str,
        default=None,
        help="OS version for CSV output (default: auto-detect)",
    )

    args = parser.parse_args()

    build_dir = Path(args.build_dir)
    if not build_dir.exists():
        print(f"Error: Build directory not found: {build_dir}", file=sys.stderr)
        sys.exit(1)

    # Auto-detect platform and OS version if needed for CSV output
    platform_name = args.platform
    os_version = args.os_version

    if args.output_csv and not platform_name:
        # Auto-detect platform
        sys_name = platform.system()
        if sys_name == "Linux":
            platform_name = "linux"
        elif sys_name == "Darwin":
            platform_name = "macos"
        elif sys_name == "Windows":
            platform_name = "windows"
        else:
            platform_name = "unknown"

    if args.output_csv and not os_version:
        # Auto-detect OS version
        if platform_name == "linux":
            try:
                import subprocess

                result = subprocess.run(
                    ["lsb_release", "-ds"],
                    capture_output=True,
                    text=True,
                    timeout=5,
                )
                if result.returncode == 0:
                    os_version = result.stdout.strip().strip('"')
                else:
                    os_version = "unknown"
            except Exception:
                os_version = "unknown"
        elif platform_name == "macos":
            os_version = platform.mac_ver()[0] or "unknown"
        elif platform_name == "windows":
            os_version = platform.win32_ver()[0] or "unknown"
        else:
            os_version = "unknown"

    # System information
    system_info = {
        "os": platform.system(),
        "arch": platform.machine(),
        "python_version": platform.python_version(),
        "cores": os.cpu_count(),
    }

    # Benchmark configurations
    benchmarks = [
        {
            "name": "qtest",
            "variants": ["native", "fc"],
            "args": [str(args.items), "4", "10"],
            "description": f"Producer-consumer queue test ({args.items // 1000}K items, 4 threads, queue size 10)",
        },
        {
            "name": "strongtest",
            "variants": ["native", "fc"],
            "args": [str(args.items), "5"],
            "description": f"Strong semantics test ({args.items // 1000}K items, queue size 5)",
        },
        {
            "name": "gil_test",
            "variants": ["native", "fc"],
            "args": [
                "4",
                str(args.items // 10),
            ],  # 4 threads, 10% of items for faster execution
            "description": f"GIL acquire/release test ({args.items // 10000}K acquisitions, 4 threads)",
        },
    ]

    impl_names = {
        "native": "native",
        "fc": "fastcond_strong",
    }

    all_results = []

    for benchmark in benchmarks:
        for variant in benchmark["variants"]:
            exe_name = f"{benchmark['name']}_{variant}"
            # Add .exe suffix on Windows
            if platform.system() == "Windows":
                exe_name += ".exe"
            exe = build_dir / exe_name

            if not exe.exists():
                print(f"Warning: {exe} not found, skipping", file=sys.stderr)
                continue

            print(f"\n{'=' * 60}", file=sys.stderr)
            print(f"Benchmark: {benchmark['name']}_{variant}", file=sys.stderr)
            print(f"{'=' * 60}", file=sys.stderr)

            result = run_benchmark_suite(
                str(exe),
                benchmark["args"],
                benchmark["name"],
                iterations=args.iterations,
                warmup=args.warmup,
                timeout=args.timeout,
            )

            if result:
                stats, per_thread_data = result

                # Configure based on test type
                if benchmark["name"] == "gil_test":
                    config = {
                        "total_items": args.items,  # total_acquisitions
                        "num_threads": 4,
                        "queue_size": 0,  # Not applicable for gil_test
                    }
                elif benchmark["name"] == "qtest":
                    config = {
                        "total_items": args.items,
                        "num_threads": 4,
                        "queue_size": int(benchmark["args"][-1]),
                    }
                else:  # strongtest
                    config = {
                        "total_items": args.items,
                        "num_threads": 1,
                        "queue_size": int(benchmark["args"][-1]),
                    }

                result_json = format_result_for_json(
                    benchmark["name"],
                    impl_names[variant],
                    benchmark["description"],
                    stats,
                    config,
                    system_info,
                    per_thread_data,
                )
                all_results.append(result_json)

    # Output results
    if args.output_csv:
        # Write CSV format compatible with run_performance_benchmarks.sh
        # Format: platform,os_version,test,variant,threads,param,iterations,elapsed_sec,throughput
        with open(args.output_csv, "w") as f:
            # Write header
            f.write(
                "platform,os_version,test,variant,threads,param,iterations,elapsed_sec,throughput\n"
            )

            # Write data rows
            for result in all_results:
                benchmark_name = result["benchmark"]
                implementation = result["implementation"]
                # Map implementation name back to variant for CSV
                variant_name = "fc" if "fastcond" in implementation else "native"

                config = result["config"]
                stats = result["results"]["statistics"]

                # Calculate elapsed time from throughput (items / items_per_sec = seconds)
                elapsed_sec = config["total_items"] / stats["mean"]

                # Extract parameters
                num_threads = config["num_threads"]
                # For gil_test, param is total_acquisitions; for others, it's queue_size
                if benchmark_name == "gil_test":
                    param = config["total_items"]  # total_acquisitions
                else:
                    param = config["queue_size"]

                f.write(
                    f"{platform_name},{os_version},{benchmark_name},{variant_name},"
                    f"{num_threads},{param},{config['total_items']},"
                    f"{elapsed_sec:.6f},{stats['mean']:.2f}\n"
                )

        print(
            f"CSV results written to {args.output_csv} ({len(all_results)} benchmarks)",
            file=sys.stderr,
        )
    else:
        # Output JSON to stdout
        print(json.dumps(all_results, indent=2))


if __name__ == "__main__":
    main()
