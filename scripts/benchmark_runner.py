#!/usr/bin/env python3
"""
Robust benchmark runner with statistical analysis.

Runs benchmarks multiple times with proper warm-up and statistical reporting.
Flexible metric extraction to support future instrumentation (false wakeups, etc).
"""

import subprocess
import re
import json
import sys
import os
import statistics
import time
from dataclasses import dataclass, asdict
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
    
    # Extended metrics
    total_false_wakeups: Optional[int] = None
    mean_false_wakeups: Optional[float] = None


def parse_test_output(output: str, test_type: str) -> BenchmarkRun:
    """
    Parse test output and extract metrics.
    
    Designed to be extensible - add new regex patterns for future metrics.
    """
    metrics = {}
    
    # Throughput (always present)
    throughput_match = re.search(r"Throughput:\s+([0-9.]+)\s+items/sec", output)
    if throughput_match:
        metrics["throughput"] = float(throughput_match.group(1))
    
    # Total time (always present)
    time_match = re.search(r"Total time:\s+([0-9.]+)\s+seconds", output)
    if time_match:
        metrics["total_time"] = float(time_match.group(1))
    
    # Latency statistics (from receiver threads)
    if test_type in ["qtest", "strongtest"]:
        # Parse first receiver's latency (representative)
        lat_match = re.search(
            r"receiver \d+ got \d+ latency avg ([0-9.e+-]+) stdev ([0-9.e+-]+) min ([0-9.e+-]+) max ([0-9.e+-]+)",
            output
        )
        if lat_match:
            metrics["latency_avg"] = float(lat_match.group(1))
            metrics["latency_stdev"] = float(lat_match.group(2))
            metrics["latency_min"] = float(lat_match.group(3))
            metrics["latency_max"] = float(lat_match.group(4))
    
    # Future: Parse false wakeup counts
    # false_wakeup_match = re.search(r"False wakeups:\s+(\d+)", output)
    # if false_wakeup_match:
    #     metrics["false_wakeups"] = int(false_wakeup_match.group(1))
    
    # Future: Parse spurious wakeup counts
    # spurious_match = re.search(r"Spurious wakeups:\s+(\d+)", output)
    # if spurious_match:
    #     metrics["spurious_wakeups"] = int(spurious_match.group(1))
    
    return BenchmarkRun(**metrics)


def run_single_benchmark(
    executable: str,
    args: List[str],
    test_type: str,
    timeout: int = 30
) -> Optional[BenchmarkRun]:
    """Run a single benchmark and return parsed results."""
    try:
        result = subprocess.run(
            [executable] + args,
            capture_output=True,
            text=True,
            timeout=timeout
        )
        
        if result.returncode != 0:
            print(f"Warning: {executable} failed with code {result.returncode}", file=sys.stderr)
            return None
        
        return parse_test_output(result.stdout, test_type)
    
    except subprocess.TimeoutExpired:
        print(f"Warning: {executable} timed out after {timeout}s", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Warning: {executable} failed: {e}", file=sys.stderr)
        return None


def calculate_statistics(runs: List[BenchmarkRun]) -> BenchmarkStatistics:
    """Calculate statistical summary from multiple runs."""
    throughputs = [r.throughput for r in runs]
    
    mean = statistics.mean(throughputs)
    stdev = statistics.stdev(throughputs) if len(throughputs) > 1 else 0.0
    cv = (stdev / mean * 100) if mean > 0 else 0.0
    
    # 95% confidence interval using t-distribution
    # For small samples, this is more accurate than normal distribution
    if len(throughputs) > 1:
        import math
        # Approximate t-value for 95% CI (good enough for n >= 3)
        t_values = {3: 4.303, 4: 3.182, 5: 2.776, 6: 2.571, 7: 2.447, 8: 2.365, 10: 2.262}
        t = t_values.get(len(throughputs), 2.0)  # Default to ~2 for larger n
        margin = t * (stdev / math.sqrt(len(throughputs)))
        ci_lower = mean - margin
        ci_upper = mean + margin
    else:
        ci_lower = ci_upper = mean
    
    stats = BenchmarkStatistics(
        mean_throughput=mean,
        stdev_throughput=stdev,
        cv_percent=cv,
        min_throughput=min(throughputs),
        max_throughput=max(throughputs),
        individual_runs=throughputs,
        confidence_interval_95=(ci_lower, ci_upper)
    )
    
    # Add latency statistics if available
    latencies = [r.latency_avg for r in runs if r.latency_avg is not None]
    if latencies:
        stats.mean_latency = statistics.mean(latencies)
        stats.stdev_latency = statistics.stdev(latencies) if len(latencies) > 1 else 0.0
    
    # Add false wakeup statistics if available
    false_wakeups = [r.false_wakeups for r in runs if r.false_wakeups is not None]
    if false_wakeups:
        stats.total_false_wakeups = sum(false_wakeups)
        stats.mean_false_wakeups = statistics.mean(false_wakeups)
    
    return stats


def run_benchmark_suite(
    executable: str,
    args: List[str],
    test_type: str,
    iterations: int = 5,
    warmup: int = 1,
    timeout: int = 30
) -> Optional[BenchmarkStatistics]:
    """
    Run benchmark multiple times with warm-up and return statistics.
    
    Args:
        executable: Path to test executable
        args: Command-line arguments
        test_type: Test type (qtest, strongtest, gil_test)
        iterations: Number of measurement runs
        warmup: Number of warm-up runs (discarded)
        timeout: Timeout per run in seconds
    """
    print(f"Running {os.path.basename(executable)}: {warmup} warm-up + {iterations} measurement runs", file=sys.stderr)
    
    # Warm-up runs
    for i in range(warmup):
        print(f"  Warm-up {i+1}/{warmup}...", end=" ", flush=True, file=sys.stderr)
        result = run_single_benchmark(executable, args, test_type, timeout)
        if result:
            print(f"{result.throughput:,.0f} items/sec", file=sys.stderr)
        else:
            print("FAILED", file=sys.stderr)
    
    # Measurement runs
    runs = []
    for i in range(iterations):
        print(f"  Run {i+1}/{iterations}...", end=" ", flush=True, file=sys.stderr)
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
    print(f"  Result: {stats.mean_throughput:,.0f} ± {stats.stdev_throughput:,.0f} items/sec (CV={stats.cv_percent:.1f}%)", file=sys.stderr)
    
    return stats


def format_result_for_json(
    benchmark_name: str,
    implementation: str,
    description: str,
    stats: BenchmarkStatistics,
    config: Dict[str, Any],
    system_info: Dict[str, Any]
) -> Dict[str, Any]:
    """Format benchmark results for JSON output (compatible with existing format)."""
    
    # Convert to microseconds for latency reporting
    latency_us = stats.mean_latency * 1e6 if stats.mean_latency else None
    latency_stdev_us = stats.stdev_latency * 1e6 if stats.stdev_latency else None
    
    return {
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
            } if latency_us else None,
            "instrumentation": {
                "false_wakeups_total": stats.total_false_wakeups,
                "false_wakeups_mean": stats.mean_false_wakeups,
            } if stats.total_false_wakeups is not None else None,
        }
    }


def main():
    """Main entry point."""
    import argparse
    import platform
    
    parser = argparse.ArgumentParser(
        description="Run benchmarks with statistical analysis"
    )
    parser.add_argument(
        "build_dir",
        help="Build directory containing test executables"
    )
    parser.add_argument(
        "--iterations", "-n",
        type=int,
        default=5,
        help="Number of measurement runs per benchmark (default: 5)"
    )
    parser.add_argument(
        "--warmup", "-w",
        type=int,
        default=1,
        help="Number of warm-up runs (default: 1)"
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=30,
        help="Timeout per run in seconds (default: 30)"
    )
    parser.add_argument(
        "--items",
        type=int,
        default=400000,
        help="Number of items for qtest/strongtest (default: 400000 for ~1s runtime)"
    )
    
    args = parser.parse_args()
    
    build_dir = Path(args.build_dir)
    if not build_dir.exists():
        print(f"Error: Build directory not found: {build_dir}", file=sys.stderr)
        sys.exit(1)
    
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
            "description": f"Producer-consumer queue test ({args.items//1000}K items, 4 threads, queue size 10)",
        },
        {
            "name": "strongtest",
            "variants": ["native", "fc"],
            "args": [str(args.items), "5"],
            "description": f"Strong semantics test ({args.items//1000}K items, queue size 5)",
        },
    ]
    
    impl_names = {
        "native": "native",
        "fc": "fastcond_strong",
    }
    
    all_results = []
    
    for benchmark in benchmarks:
        for variant in benchmark["variants"]:
            exe = build_dir / f"{benchmark['name']}_{variant}"
            
            if not exe.exists():
                print(f"Warning: {exe} not found, skipping", file=sys.stderr)
                continue
            
            print(f"\n{'='*60}", file=sys.stderr)
            print(f"Benchmark: {benchmark['name']}_{variant}", file=sys.stderr)
            print(f"{'='*60}", file=sys.stderr)
            
            stats = run_benchmark_suite(
                str(exe),
                benchmark["args"],
                benchmark["name"],
                iterations=args.iterations,
                warmup=args.warmup,
                timeout=args.timeout
            )
            
            if stats:
                config = {
                    "total_items": args.items,
                    "num_threads": 4 if benchmark["name"] == "qtest" else 1,
                    "queue_size": int(benchmark["args"][-1]),
                }
                
                result = format_result_for_json(
                    benchmark["name"],
                    impl_names[variant],
                    benchmark["description"],
                    stats,
                    config,
                    system_info
                )
                all_results.append(result)
    
    # Output JSON
    print(json.dumps(all_results, indent=2))


if __name__ == "__main__":
    main()
