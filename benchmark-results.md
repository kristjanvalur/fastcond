# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 566,732.26 | +0.5% |
| native | 563,739.23 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 8.86 | 0.04 | 113.16 | 9.03 |
| fastcond_strong | 9.00 | 0.04 | 1145.13 | 11.36 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 564,593.82 | baseline |
| fastcond_strong | 417,233.35 | -26.1% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 3.94 | 0.03 | 6746.92 | 32.47 |
| fastcond_strong | 5.88 | 0.03 | 10549.05 | 28.97 |
