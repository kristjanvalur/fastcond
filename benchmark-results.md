# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 665,326.74 | baseline |
| fastcond_strong | 457,023.60 | -31.3% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.13 | 0.08 | 81.35 | 8.30 |
| fastcond_strong | 12.54 | 0.05 | 100.55 | 13.29 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,226,271.79 | +10.5% |
| native | 1,109,999.88 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.20 | 0.07 | 1184.50 | 29.35 |
| fastcond_strong | 2.28 | 0.08 | 1732.55 | 37.13 |
