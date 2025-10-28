# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 716,850.15 | baseline |
| fastcond_strong | 692,015.39 | -3.5% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.20 | 0.04 | 303.45 | 7.94 |
| fastcond_strong | 7.13 | 0.04 | 152.96 | 7.91 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,162,959.69 | +3.0% |
| native | 1,129,171.52 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 1.88 | 0.05 | 2846.09 | 25.84 |
| fastcond_strong | 1.95 | 0.04 | 5094.17 | 34.45 |
