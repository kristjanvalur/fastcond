# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 685,941.89 | baseline |
| fastcond_strong | 669,591.22 | -2.4% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 8.16 | 0.04 | 302.24 | 9.87 |
| fastcond_strong | 6.77 | 0.05 | 118.51 | 7.32 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 1,045,130.03 | baseline |
| fastcond_strong | 1,006,642.93 | -3.7% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.35 | 0.04 | 7967.23 | 53.09 |
| fastcond_strong | 2.56 | 0.04 | 4257.05 | 41.84 |
