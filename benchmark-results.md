# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 738,418.55 | +6.6% |
| native | 693,010.35 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.58 | 0.05 | 1349.98 | 13.78 |
| fastcond_strong | 6.89 | 0.05 | 111.54 | 7.41 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,103,841.18 | +0.5% |
| native | 1,098,183.24 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.01 | 0.04 | 5009.70 | 38.95 |
| fastcond_strong | 2.58 | 0.05 | 4011.57 | 36.68 |
