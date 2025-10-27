# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 658,650.80 | +98.4% |
| native | 331,936.91 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 14.74 | 0.07 | 314.77 | 25.07 |
| fastcond_strong | 7.30 | 0.08 | 192.57 | 12.06 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,131,000.64 | +12.7% |
| native | 1,003,776.31 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.75 | 0.07 | 924.77 | 33.05 |
| fastcond_strong | 2.72 | 0.07 | 1927.88 | 46.24 |
