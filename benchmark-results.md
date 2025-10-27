# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 647,163.53 | baseline |
| fastcond_strong | 552,252.11 | -14.7% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 8.16 | 0.06 | 88.05 | 9.58 |
| fastcond_strong | 9.42 | 0.07 | 83.36 | 10.84 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,276,227.43 | +5.9% |
| native | 1,204,929.61 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.36 | 0.04 | 1399.98 | 35.47 |
| fastcond_strong | 1.69 | 0.07 | 939.88 | 24.84 |
