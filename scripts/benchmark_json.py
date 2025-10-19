#!/usr/bin/env python3
"""
Performance benchmark runner and JSON converter for fastcond tests.

This script runs the performance tests and converts the output to structured JSON
for analysis and visualization.
"""

import subprocess
import re
import json
import sys
import os
from datetime import datetime
import platform


def parse_qtest_output(output):
    """Parse qtest output and extract metrics."""
    results = {"per_thread": [], "overall": {}}

    # Parse per-receiver lines
    # Example: receiver 0 got 2425 latency avg 1.037377e-05 stdev 1.688608e-05 min 4.600000e-08 max 1.487380e-04
    receiver_pattern = r"receiver (\d+) got (\d+) latency avg ([0-9.e+-]+) stdev ([0-9.e+-]+) min ([0-9.e+-]+) max ([0-9.e+-]+)"

    for match in re.finditer(receiver_pattern, output):
        thread_id, items, avg, stdev, min_lat, max_lat = match.groups()
        results["per_thread"].append(
            {
                "thread_id": int(thread_id),
                "thread_type": "receiver",
                "items": int(items),
                "avg_latency_sec": float(avg),
                "stdev_latency_sec": float(stdev),
                "min_latency_sec": float(min_lat),
                "max_latency_sec": float(max_lat),
            }
        )

    # Parse overall statistics
    # Total items: 10000
    # Threads: 4 senders, 4 receivers
    # Queue size: 10
    # Total time: 0.022270 seconds
    # Throughput: 449039.44 items/sec

    total_items_match = re.search(r"Total items: (\d+)", output)
    if total_items_match:
        results["overall"]["total_items"] = int(total_items_match.group(1))

    threads_match = re.search(r"Threads: (\d+) senders, (\d+) receivers", output)
    if threads_match:
        results["overall"]["num_senders"] = int(threads_match.group(1))
        results["overall"]["num_receivers"] = int(threads_match.group(2))

    queue_size_match = re.search(r"Queue size: (\d+)", output)
    if queue_size_match:
        results["overall"]["queue_size"] = int(queue_size_match.group(1))

    total_time_match = re.search(r"Total time: ([0-9.]+) seconds", output)
    if total_time_match:
        results["overall"]["total_time_sec"] = float(total_time_match.group(1))

    throughput_match = re.search(r"Throughput: ([0-9.]+) items/sec", output)
    if throughput_match:
        results["overall"]["throughput_items_per_sec"] = float(
            throughput_match.group(1)
        )

    return results


def parse_strongtest_output(output):
    """Parse strongtest output and extract metrics."""
    results = {"per_thread": [], "overall": {}}

    # Parse receiver line
    # Example: receiver 0 got 10000 latency avg 1.914962e-06 stdev 2.627846e-05 min 3.600000e-08 max 1.173501e-03
    receiver_pattern = r"receiver (\d+) got (\d+) latency avg ([0-9.e+-]+) stdev ([0-9.e+-]+) min ([0-9.e+-]+) max ([0-9.e+-]+)"

    match = re.search(receiver_pattern, output)
    if match:
        thread_id, items, avg, stdev, min_lat, max_lat = match.groups()
        results["per_thread"].append(
            {
                "thread_id": int(thread_id),
                "thread_type": "receiver",
                "items": int(items),
                "avg_latency_sec": float(avg),
                "stdev_latency_sec": float(stdev),
                "min_latency_sec": float(min_lat),
                "max_latency_sec": float(max_lat),
            }
        )

    # Parse overall statistics (same as qtest)
    total_items_match = re.search(r"Total items: (\d+)", output)
    if total_items_match:
        results["overall"]["total_items"] = int(total_items_match.group(1))

    threads_match = re.search(r"Threads: (\d+) senders, (\d+) receivers", output)
    if threads_match:
        results["overall"]["num_senders"] = int(threads_match.group(1))
        results["overall"]["num_receivers"] = int(threads_match.group(2))

    queue_size_match = re.search(r"Queue size: (\d+)", output)
    if queue_size_match:
        results["overall"]["queue_size"] = int(queue_size_match.group(1))

    total_time_match = re.search(r"Total time: ([0-9.]+) seconds", output)
    if total_time_match:
        results["overall"]["total_time_sec"] = float(total_time_match.group(1))

    throughput_match = re.search(r"Throughput: ([0-9.]+) items/sec", output)
    if throughput_match:
        results["overall"]["throughput_items_per_sec"] = float(
            throughput_match.group(1)
        )

    return results

    return results


def run_benchmark(executable, args, test_name, implementation):
    """Run a single benchmark and return JSON results."""
    try:
        result = subprocess.run(
            [executable] + args, capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            return {
                "error": f"Command failed with return code {result.returncode}",
                "stderr": result.stderr,
            }

        # Parse output based on test type
        if "qtest" in executable:
            parsed = parse_qtest_output(result.stdout)
        elif "strongtest" in executable:
            parsed = parse_strongtest_output(result.stdout)
        else:
            return {"error": f"Unknown test type: {executable}"}

        # Add metadata
        result_dict = {
            "benchmark": test_name,
            "implementation": implementation,
            "timestamp": datetime.now().isoformat(),
            "system": {
                "os": platform.system(),
                "arch": platform.machine(),
                "python_version": platform.python_version(),
                "cores": os.cpu_count(),
            },
            "config": parsed.get("overall", {}),
            "results": {"overall": parsed.get("overall", {})},
        }

        # Add per-thread results for qtest and strongtest
        if "per_thread" in parsed:
            result_dict["results"]["per_thread"] = parsed["per_thread"]

        # Add fairness statistics for GIL tests
        if "fairness_stats" in parsed:
            result_dict["results"]["fairness_stats"] = parsed["fairness_stats"]

        return result_dict

    except subprocess.TimeoutExpired:
        return {"error": "Benchmark timed out after 30 seconds"}
    except Exception as e:
        return {"error": str(e)}


def main():
    """Run all benchmarks and output JSON."""
    if len(sys.argv) < 2:
        print("Usage: python3 benchmark_json.py <build_directory>")
        print("Example: python3 scripts/benchmark_json.py build")
        sys.exit(1)

    build_dir = sys.argv[1]

    # Test configurations
    benchmarks = [
        {
            "name": "qtest",
            "variants": ["pt", "fc", "wcond"],
            "args": ["10000", "4", "10"],
            "description": "Producer-consumer queue test (10K items, 4 threads, queue size 10)",
        },
        {
            "name": "strongtest",
            "variants": ["pt", "fc"],  # Don't run wcond (will deadlock)
            "args": ["10000", "5"],
            "description": "Strong semantics test (10K items, queue size 5)",
        },
        # Note: GIL tests are run separately via run_gil_fairness_analysis.sh
        # They have different JSON structure and are published to gil-fairness.html
    ]

    impl_names = {
        "pt": "pthread",
        "fc": "fastcond_strong",
        "wcond": "fastcond_weak",
        # fc_unfair and fc_naive are handled by GIL-specific scripts
    }

    all_results = []

    for benchmark in benchmarks:
        for variant in benchmark["variants"]:
            exe = os.path.join(build_dir, f"{benchmark['name']}_{variant}")

            if not os.path.exists(exe):
                print(f"Warning: {exe} not found, skipping", file=sys.stderr)
                continue

            impl_name = impl_names.get(variant, variant)
            result = run_benchmark(exe, benchmark["args"], benchmark["name"], impl_name)
            result["description"] = benchmark["description"]

            all_results.append(result)

    # Output JSON
    print(json.dumps(all_results, indent=2))


if __name__ == "__main__":
    main()
