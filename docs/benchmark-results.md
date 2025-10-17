# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 640,088.81 | +45.5% |
| pthread | 439,862.18 | baseline |
| fastcond_weak | 388,728.81 | -11.6% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.91 | 0.05 | 141.03 | 15.98 |
| fastcond_strong | 7.64 | 0.04 | 169.96 | 13.79 |
| fastcond_weak | 11.14 | 0.05 | 158.92 | 17.24 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,139,642.05 | +2.4% |
| pthread | 1,112,967.29 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 3.02 | 0.04 | 1609.95 | 50.09 |
| fastcond_strong | 2.66 | 0.04 | 1382.67 | 37.77 |
