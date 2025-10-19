# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 477,031.03 | +16.0% |
| pthread | 411,163.42 | baseline |
| fastcond_weak | 292,670.52 | -28.8% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.85 | 0.05 | 169.67 | 15.55 |
| fastcond_strong | 10.75 | 0.05 | 196.24 | 19.68 |
| fastcond_weak | 17.01 | 0.05 | 220.17 | 31.41 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| pthread | 1,147,179.19 | baseline |
| fastcond_strong | 1,037,941.43 | -9.5% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.06 | 0.05 | 1653.53 | 36.50 |
| fastcond_strong | 2.04 | 0.03 | 1697.95 | 36.36 |
