# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 726,666.91 | +5.6% |
| native | 688,283.37 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.93 | 0.03 | 130.64 | 9.32 |
| fastcond_strong | 7.57 | 0.04 | 240.37 | 8.93 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,072,690.58 | +0.3% |
| native | 1,069,428.51 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.40 | 0.04 | 10998.46 | 44.11 |
| fastcond_strong | 1.99 | 0.03 | 5002.33 | 31.86 |
