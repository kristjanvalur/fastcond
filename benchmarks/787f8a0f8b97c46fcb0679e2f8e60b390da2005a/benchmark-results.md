# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 656,237.65 | +30.3% |
| fastcond_weak | 578,013.10 | +14.7% |
| pthread | 503,755.02 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.34 | 0.06 | 97.35 | 12.69 |
| fastcond_strong | 7.52 | 0.06 | 77.92 | 8.42 |
| fastcond_weak | 8.93 | 0.07 | 94.79 | 10.19 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,313,162.97 | +47.1% |
| pthread | 892,824.70 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 3.24 | 0.07 | 1041.58 | 33.40 |
| fastcond_strong | 2.56 | 0.06 | 1658.45 | 39.75 |
