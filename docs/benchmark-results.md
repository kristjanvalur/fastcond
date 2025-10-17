# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 688,199.46 | +70.9% |
| pthread | 402,702.52 | baseline |
| fastcond_weak | 376,926.23 | -6.4% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 12.34 | 0.05 | 249.67 | 18.45 |
| fastcond_strong | 6.66 | 0.04 | 145.93 | 10.96 |
| fastcond_weak | 13.02 | 0.04 | 212.45 | 20.41 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,119,224.49 | +5.3% |
| pthread | 1,062,507.76 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.18 | 0.04 | 476.26 | 25.84 |
| fastcond_strong | 2.41 | 0.04 | 1135.60 | 35.94 |
