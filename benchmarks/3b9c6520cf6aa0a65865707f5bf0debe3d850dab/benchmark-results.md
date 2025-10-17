# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 616,364.20 | +27.2% |
| fastcond_weak | 557,077.83 | +14.9% |
| pthread | 484,699.33 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.64 | 0.05 | 97.40 | 12.69 |
| fastcond_strong | 9.44 | 0.06 | 85.82 | 10.45 |
| fastcond_weak | 9.09 | 0.05 | 106.39 | 10.48 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,333,799.81 | +83.5% |
| pthread | 726,861.14 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.46 | 0.05 | 1072.91 | 24.59 |
| fastcond_strong | 1.14 | 0.06 | 1393.29 | 21.89 |
