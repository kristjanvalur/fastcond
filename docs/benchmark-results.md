# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 572,089.71 | +41.4% |
| fastcond_weak | 408,514.69 | +1.0% |
| pthread | 404,549.79 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 11.61 | 0.05 | 130.88 | 16.83 |
| fastcond_strong | 8.75 | 0.05 | 186.13 | 15.47 |
| fastcond_weak | 12.60 | 0.05 | 214.83 | 19.40 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,125,094.03 | +1.0% |
| pthread | 1,114,241.01 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.20 | 0.04 | 1646.25 | 39.92 |
| fastcond_strong | 2.66 | 0.04 | 1423.60 | 45.75 |
