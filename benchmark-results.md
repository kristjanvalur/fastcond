# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 737,036.79 | baseline |
| fastcond_strong | 716,121.47 | -2.8% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.08 | 0.04 | 114.10 | 7.81 |
| fastcond_strong | 7.17 | 0.04 | 118.26 | 7.66 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 1,067,238.76 | baseline |
| fastcond_strong | 989,630.23 | -7.3% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.23 | 0.05 | 3998.45 | 28.98 |
| fastcond_strong | 2.26 | 0.05 | 2916.46 | 28.43 |
