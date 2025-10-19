#!/usr/bin/env python3
"""
GIL Fairness Benchmark Runner

This script runs the three GIL fairness modes and generates JSON results.
Separated from the main benchmark script to maintain clean data structures.
"""

import os
import sys
import json
import re
import subprocess
import platform
from datetime import datetime


def parse_gil_output(output):
    """Parse GIL test output and extract metrics."""
    results = {"fairness_stats": {}, "overall": {}}

    # Parse thread acquisition counts
    # Example:   Thread 0: 1250 acquisitions
    thread_pattern = r"Thread (\d+): (\d+) acquisitions"
    thread_acquisitions = []

    for match in re.finditer(thread_pattern, output):
        thread_id, count = match.groups()
        thread_acquisitions.append(
            {"thread_id": int(thread_id), "acquisitions": int(count)}
        )

    results["fairness_stats"]["thread_acquisitions"] = thread_acquisitions

    # Parse fairness metrics
    # Coefficient of variation: 0.033
    cv_match = re.search(r"Coefficient of variation: ([0-9.]+)", output)
    if cv_match:
        results["fairness_stats"]["coefficient_of_variation"] = float(cv_match.group(1))

    # Thread transitions: 4999 out of 5000 acquisitions (100.0%)
    transitions_match = re.search(
        r"Thread transitions: (\d+) out of (\d+) acquisitions \(([0-9.]+)%\)", output
    )
    if transitions_match:
        transitions, total_acq, pct = transitions_match.groups()
        results["fairness_stats"]["thread_transitions"] = int(transitions)
        results["fairness_stats"]["total_acquisitions"] = int(total_acq)
        results["fairness_stats"]["transition_percentage"] = float(pct)

    # Consecutive re-acquisitions: 0 (0.0%)
    consec_match = re.search(
        r"Consecutive re-acquisitions: (\d+) \(([0-9.]+)%\)", output
    )
    if consec_match:
        consec_count, consec_pct = consec_match.groups()
        results["fairness_stats"]["consecutive_reacquisitions"] = int(consec_count)
        results["fairness_stats"]["consecutive_percentage"] = float(consec_pct)

    # Fairness score: 0.0 (lower is better)
    score_match = re.search(r"Fairness score: ([0-9.]+)", output)
    if score_match:
        results["fairness_stats"]["fairness_score"] = float(score_match.group(1))

    # Operations per second: 5539
    ops_match = re.search(r"Operations per second: (\d+)", output)
    if ops_match:
        results["overall"]["operations_per_sec"] = int(ops_match.group(1))

    # Average latency per operation: 180.5 μs
    latency_match = re.search(r"Average latency per operation: ([0-9.]+) μs", output)
    if latency_match:
        results["overall"]["avg_latency_us"] = float(latency_match.group(1))

    # Backend and fairness info
    backend_match = re.search(r"Backend: ([^\n]+)", output)
    if backend_match:
        results["overall"]["backend"] = backend_match.group(1).strip()

    fairness_enabled_match = re.search(r"Fairness: ([^\n]+)", output)
    if fairness_enabled_match:
        fairness_status = fairness_enabled_match.group(1).strip()
        results["overall"]["fairness_enabled"] = "ENABLED" in fairness_status

    return results


def run_gil_benchmark(executable, test_name, implementation, fairness_mode):
    """Run a single GIL benchmark and return JSON results."""

    try:
        result = subprocess.run(
            [executable], capture_output=True, text=True, timeout=60
        )

        if result.returncode != 0:
            return {
                "error": f"Execution failed with code {result.returncode}",
                "stderr": result.stderr,
            }

        # Parse GIL-specific output
        parsed = parse_gil_output(result.stdout)

        # Add metadata
        result_dict = {
            "benchmark": test_name,
            "implementation": implementation,
            "fairness_mode": fairness_mode,
            "timestamp": datetime.now().isoformat(),
            "system": {
                "os": platform.system(),
                "arch": platform.machine(),
                "python_version": platform.python_version(),
                "cores": os.cpu_count(),
            },
            "config": {
                "operations_per_sec": parsed["overall"].get("operations_per_sec", 0),
                "avg_latency_us": parsed["overall"].get("avg_latency_us", 0.0),
                "backend": parsed["overall"].get("backend", "unknown"),
                "fairness_enabled": parsed["overall"].get("fairness_enabled", False),
            },
            "results": parsed,
        }

        return result_dict

    except subprocess.TimeoutExpired:
        return {"error": "Benchmark timed out after 60 seconds"}
    except Exception as e:
        return {"error": f"Execution error: {str(e)}"}


def main():
    if len(sys.argv) != 2:
        print("Usage: gil_benchmark_json.py <build_directory>", file=sys.stderr)
        sys.exit(1)

    build_dir = sys.argv[1]

    # GIL test configurations
    gil_tests = [
        {
            "executable": "gil_test_fc",
            "implementation": "fastcond_strong",
            "fairness_mode": "fair",
        },
        {
            "executable": "gil_test_fc_unfair",
            "implementation": "fastcond_unfair",
            "fairness_mode": "unfair",
        },
        {
            "executable": "gil_test_fc_naive",
            "implementation": "fastcond_naive",
            "fairness_mode": "naive",
        },
    ]

    all_results = []

    for test_config in gil_tests:
        exe_path = os.path.join(build_dir, test_config["executable"])

        if not os.path.exists(exe_path):
            print(f"Warning: {exe_path} not found, skipping", file=sys.stderr)
            continue

        result = run_gil_benchmark(
            exe_path,
            "gil_test",
            test_config["implementation"],
            test_config["fairness_mode"],
        )

        all_results.append(result)

    # Output JSON
    print(json.dumps(all_results, indent=2))


if __name__ == "__main__":
    main()
