# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| pthread | 568,862.19 | baseline |
| fastcond_strong | 532,263.95 | -6.4% |
| fastcond_weak | 486,105.48 | -14.5% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 8.72 | 0.05 | 87.77 | 10.50 |
| fastcond_strong | 10.07 | 0.07 | 69.19 | 10.82 |
| fastcond_weak | 10.33 | 0.06 | 100.97 | 11.78 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,336,285.63 | +18.5% |
| pthread | 1,127,736.33 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.29 | 0.07 | 1822.61 | 40.08 |
| fastcond_strong | 2.07 | 0.05 | 1009.25 | 37.34 |
