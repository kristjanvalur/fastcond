# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 579,095.50 | +2.7% |
| pthread | 564,009.24 | baseline |
| fastcond_weak | 556,597.75 | -1.3% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 9.12 | 0.05 | 85.30 | 10.39 |
| fastcond_strong | 8.57 | 0.04 | 69.40 | 8.82 |
| fastcond_weak | 8.97 | 0.04 | 93.35 | 10.75 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,312,151.88 | +82.0% |
| pthread | 720,915.56 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 3.83 | 0.10 | 887.45 | 30.07 |
| fastcond_strong | 1.76 | 0.09 | 1004.00 | 28.83 |
