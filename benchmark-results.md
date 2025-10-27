# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 773,766.26 | +11.5% |
| native | 693,747.53 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 8.26 | 0.05 | 1744.48 | 12.79 |
| fastcond_strong | 7.36 | 0.05 | 1132.44 | 9.57 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,172,093.84 | +1.4% |
| native | 1,156,341.74 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 1.90 | 0.06 | 3010.92 | 30.61 |
| fastcond_strong | 2.22 | 0.03 | 7381.95 | 49.82 |
