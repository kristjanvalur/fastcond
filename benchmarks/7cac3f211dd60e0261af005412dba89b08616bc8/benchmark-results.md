# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_weak | 608,200.81 | +24.7% |
| fastcond_strong | 551,436.69 | +13.1% |
| pthread | 487,634.35 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.29 | 0.06 | 99.81 | 12.68 |
| fastcond_strong | 9.81 | 0.06 | 99.28 | 10.50 |
| fastcond_weak | 8.82 | 0.09 | 78.94 | 10.52 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,311,245.66 | +25.3% |
| pthread | 1,046,336.05 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.57 | 0.06 | 1380.04 | 35.73 |
| fastcond_strong | 1.31 | 0.08 | 1004.14 | 23.66 |
