# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 728,464.96 | baseline |
| fastcond_strong | 701,257.26 | -3.7% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.52 | 0.04 | 852.61 | 9.28 |
| fastcond_strong | 7.24 | 0.04 | 117.49 | 8.05 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 1,128,736.96 | baseline |
| fastcond_strong | 1,085,231.41 | -3.9% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.18 | 0.04 | 4955.03 | 41.27 |
| fastcond_strong | 2.01 | 0.04 | 10019.04 | 51.35 |
