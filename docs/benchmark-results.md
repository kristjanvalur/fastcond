# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 569,193.85 | +21.0% |
| pthread | 470,415.12 | baseline |
| fastcond_weak | 358,476.72 | -23.8% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.18 | 0.05 | 135.39 | 15.90 |
| fastcond_strong | 9.05 | 0.05 | 134.90 | 14.15 |
| fastcond_weak | 12.97 | 0.05 | 204.81 | 21.50 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| pthread | 1,178,049.17 | baseline |
| fastcond_strong | 1,063,515.94 | -9.7% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.15 | 0.05 | 1625.20 | 45.58 |
| fastcond_strong | 2.74 | 0.03 | 1271.28 | 38.75 |
