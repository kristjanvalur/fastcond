# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 577,906.14 | +31.9% |
| fastcond_weak | 490,652.14 | +12.0% |
| pthread | 438,259.90 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.60 | 0.05 | 123.43 | 15.15 |
| fastcond_strong | 8.18 | 0.04 | 106.50 | 12.40 |
| fastcond_weak | 10.93 | 0.04 | 118.24 | 16.22 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,173,385.69 | +19.7% |
| pthread | 980,596.74 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.57 | 0.04 | 1565.10 | 33.57 |
| fastcond_strong | 1.97 | 0.04 | 1550.20 | 37.16 |
