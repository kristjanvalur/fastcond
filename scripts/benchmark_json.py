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
    results = {
        'per_thread': [],
        'overall': {}
    }
    
    # Parse per-receiver lines
    # Example: receiver 0 got 2425 latency avg 1.037377e-05 stdev 1.688608e-05 min 4.600000e-08 max 1.487380e-04
    receiver_pattern = r'receiver (\d+) got (\d+) latency avg ([0-9.e+-]+) stdev ([0-9.e+-]+) min ([0-9.e+-]+) max ([0-9.e+-]+)'
    
    for match in re.finditer(receiver_pattern, output):
        thread_id, items, avg, stdev, min_lat, max_lat = match.groups()
        results['per_thread'].append({
            'thread_id': int(thread_id),
            'thread_type': 'receiver',
            'items': int(items),
            'avg_latency_sec': float(avg),
            'stdev_latency_sec': float(stdev),
            'min_latency_sec': float(min_lat),
            'max_latency_sec': float(max_lat)
        })
    
    # Parse overall statistics
    # Total items: 10000
    # Threads: 4 senders, 4 receivers
    # Queue size: 10
    # Total time: 0.022270 seconds
    # Throughput: 449039.44 items/sec
    
    total_items_match = re.search(r'Total items: (\d+)', output)
    if total_items_match:
        results['overall']['total_items'] = int(total_items_match.group(1))
    
    threads_match = re.search(r'Threads: (\d+) senders, (\d+) receivers', output)
    if threads_match:
        results['overall']['num_senders'] = int(threads_match.group(1))
        results['overall']['num_receivers'] = int(threads_match.group(2))
    
    queue_size_match = re.search(r'Queue size: (\d+)', output)
    if queue_size_match:
        results['overall']['queue_size'] = int(queue_size_match.group(1))
    
    total_time_match = re.search(r'Total time: ([0-9.]+) seconds', output)
    if total_time_match:
        results['overall']['total_time_sec'] = float(total_time_match.group(1))
    
    throughput_match = re.search(r'Throughput: ([0-9.]+) items/sec', output)
    if throughput_match:
        results['overall']['throughput_items_per_sec'] = float(throughput_match.group(1))
    
    return results

def parse_strongtest_output(output):
    """Parse strongtest output and extract metrics."""
    results = {
        'per_thread': [],
        'overall': {}
    }
    
    # Parse receiver line
    # Example: receiver 0 got 10000 latency avg 1.914962e-06 stdev 2.627846e-05 min 3.600000e-08 max 1.173501e-03
    receiver_pattern = r'receiver (\d+) got (\d+) latency avg ([0-9.e+-]+) stdev ([0-9.e+-]+) min ([0-9.e+-]+) max ([0-9.e+-]+)'
    
    match = re.search(receiver_pattern, output)
    if match:
        thread_id, items, avg, stdev, min_lat, max_lat = match.groups()
        results['per_thread'].append({
            'thread_id': int(thread_id),
            'thread_type': 'receiver',
            'items': int(items),
            'avg_latency_sec': float(avg),
            'stdev_latency_sec': float(stdev),
            'min_latency_sec': float(min_lat),
            'max_latency_sec': float(max_lat)
        })
    
    # Parse overall statistics (same as qtest)
    total_items_match = re.search(r'Total items: (\d+)', output)
    if total_items_match:
        results['overall']['total_items'] = int(total_items_match.group(1))
    
    threads_match = re.search(r'Threads: (\d+) senders, (\d+) receivers', output)
    if threads_match:
        results['overall']['num_senders'] = int(threads_match.group(1))
        results['overall']['num_receivers'] = int(threads_match.group(2))
    
    queue_size_match = re.search(r'Queue size: (\d+)', output)
    if queue_size_match:
        results['overall']['queue_size'] = int(queue_size_match.group(1))
    
    total_time_match = re.search(r'Total time: ([0-9.]+) seconds', output)
    if total_time_match:
        results['overall']['total_time_sec'] = float(total_time_match.group(1))
    
    throughput_match = re.search(r'Throughput: ([0-9.]+) items/sec', output)
    if throughput_match:
        results['overall']['throughput_items_per_sec'] = float(throughput_match.group(1))
    
    return results


def parse_gil_output(output):
    """Parse GIL test output and extract metrics."""
    results = {
        'fairness_stats': {},
        'overall': {}
    }
    
    # Parse thread acquisition counts
    # Example:   Thread 0: 1250 acquisitions
    thread_pattern = r'Thread (\d+): (\d+) acquisitions'
    thread_acquisitions = []
    
    for match in re.finditer(thread_pattern, output):
        thread_id, count = match.groups()
        thread_acquisitions.append({
            'thread_id': int(thread_id),
            'acquisitions': int(count)
        })
    
    results['fairness_stats']['thread_acquisitions'] = thread_acquisitions
    
    # Parse fairness metrics
    # Coefficient of variation: 0.033
    cv_match = re.search(r'Coefficient of variation: ([0-9.]+)', output)
    if cv_match:
        results['fairness_stats']['coefficient_of_variation'] = float(cv_match.group(1))
    
    # Thread transitions: 4999 out of 5000 acquisitions (100.0%)
    transitions_match = re.search(r'Thread transitions: (\d+) out of (\d+) acquisitions \(([0-9.]+)%\)', output)
    if transitions_match:
        transitions, total_acq, pct = transitions_match.groups()
        results['fairness_stats']['thread_transitions'] = int(transitions)
        results['fairness_stats']['total_acquisitions'] = int(total_acq)
        results['fairness_stats']['transition_percentage'] = float(pct)
    
    # Consecutive re-acquisitions: 0 (0.0%)
    consec_match = re.search(r'Consecutive re-acquisitions: (\d+) \(([0-9.]+)%\)', output)
    if consec_match:
        consec_count, consec_pct = consec_match.groups()
        results['fairness_stats']['consecutive_reacquisitions'] = int(consec_count)
        results['fairness_stats']['consecutive_percentage'] = float(consec_pct)
    
    # Fairness score: 0.0 (lower is better)
    score_match = re.search(r'Fairness score: ([0-9.]+)', output)
    if score_match:
        results['fairness_stats']['fairness_score'] = float(score_match.group(1))
    
    # Operations per second: 5539
    ops_match = re.search(r'Operations per second: (\d+)', output)
    if ops_match:
        results['overall']['operations_per_sec'] = int(ops_match.group(1))
    
    # Average latency per operation: 180.5 μs
    latency_match = re.search(r'Average latency per operation: ([0-9.]+) μs', output)
    if latency_match:
        results['overall']['avg_latency_us'] = float(latency_match.group(1))
    
    # Backend and fairness info
    backend_match = re.search(r'Backend: ([^\n]+)', output)
    if backend_match:
        results['overall']['backend'] = backend_match.group(1).strip()
    
    fairness_enabled_match = re.search(r'Fairness: ([^\n]+)', output)
    if fairness_enabled_match:
        fairness_status = fairness_enabled_match.group(1).strip()
        results['overall']['fairness_enabled'] = 'ENABLED' in fairness_status
    
    return results


def run_benchmark(executable, args, test_name, implementation):
    """Run a single benchmark and return JSON results."""
    try:
        result = subprocess.run(
            [executable] + args,
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode != 0:
            return {
                'error': f"Command failed with return code {result.returncode}",
                'stderr': result.stderr
            }
        
        # Parse output based on test type
        if 'qtest' in executable:
            parsed = parse_qtest_output(result.stdout)
        elif 'strongtest' in executable:
            parsed = parse_strongtest_output(result.stdout)
        elif 'gil_test' in executable:
            parsed = parse_gil_output(result.stdout)
        else:
            return {'error': 'Unknown test type'}
        
        # Add metadata
        result_dict = {
            'benchmark': test_name,
            'implementation': implementation,
            'timestamp': datetime.now().isoformat(),
            'system': {
                'os': platform.system(),
                'arch': platform.machine(),
                'python_version': platform.python_version(),
                'cores': os.cpu_count()
            },
            'config': parsed.get('overall', {}),
            'results': {
                'overall': parsed.get('overall', {})
            }
        }
        
        # Add per-thread results for qtest and strongtest
        if 'per_thread' in parsed:
            result_dict['results']['per_thread'] = parsed['per_thread']
        
        # Add fairness statistics for GIL tests
        if 'fairness_stats' in parsed:
            result_dict['results']['fairness_stats'] = parsed['fairness_stats']
        
        return result_dict
    
    except subprocess.TimeoutExpired:
        return {'error': 'Benchmark timed out after 30 seconds'}
    except Exception as e:
        return {'error': str(e)}

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
            'name': 'qtest',
            'variants': ['pt', 'fc', 'wcond'],
            'args': ['10000', '4', '10'],
            'description': 'Producer-consumer queue test (10K items, 4 threads, queue size 10)'
        },
        {
            'name': 'strongtest',
            'variants': ['pt', 'fc'],  # Don't run wcond (will deadlock)
            'args': ['10000', '5'],
            'description': 'Strong semantics test (10K items, queue size 5)'
        },
        {
            'name': 'gil_test',
            'variants': ['fc', 'fc_unfair', 'fc_naive'],  # Include all three modes for comparison
            'args': [],  # Use default parameters (4 threads, 10K operations)
            'description': 'Global Interpreter Lock fairness test (default: 4 threads, 10K operations)'
        }
    ]
    
    impl_names = {
        'pt': 'pthread',
        'fc': 'fastcond_strong',
        'wcond': 'fastcond_weak',
        'fc_unfair': 'fastcond_unfair',
        'fc_naive': 'fastcond_naive'
    }
    
    all_results = []
    
    for benchmark in benchmarks:
        for variant in benchmark['variants']:
            exe = os.path.join(build_dir, f"{benchmark['name']}_{variant}")
            
            if not os.path.exists(exe):
                print(f"Warning: {exe} not found, skipping", file=sys.stderr)
                continue
            
            impl_name = impl_names.get(variant, variant)
            result = run_benchmark(exe, benchmark['args'], benchmark['name'], impl_name)
            result['description'] = benchmark['description']
            
            # Add fairness-specific metadata for GIL tests
            if benchmark['name'] == 'gil_test':
                if variant == 'fc':
                    result['fairness_mode'] = 'fair'
                elif variant == 'fc_unfair':
                    result['fairness_mode'] = 'unfair'
                elif variant == 'fc_naive':
                    result['fairness_mode'] = 'naive'
            
            all_results.append(result)
    
    # Output JSON
    print(json.dumps(all_results, indent=2))

if __name__ == '__main__':
    main()
