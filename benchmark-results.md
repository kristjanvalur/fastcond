# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 633,918.00 | +51.6% |
| native | 418,171.35 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 11.44 | 0.09 | 99.17 | 13.69 |
| fastcond_strong | 7.71 | 0.04 | 80.93 | 9.11 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,338,983.67 | +26.0% |
| native | 1,062,731.44 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 1.52 | 0.06 | 1045.80 | 16.31 |
| fastcond_strong | 1.53 | 0.08 | 982.48 | 24.85 |
