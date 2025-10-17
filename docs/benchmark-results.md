# Performance Benchmark Results

Generated from: `/home/kristjan/git/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 565,960.17 | +26.5% |
| pthread | 447,387.06 | baseline |
| fastcond_weak | 427,870.03 | -4.4% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 10.47 | 0.05 | 154.30 | 15.85 |
| fastcond_strong | 9.13 | 0.05 | 118.27 | 13.76 |
| fastcond_weak | 11.64 | 0.05 | 172.81 | 18.30 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| pthread | 1,131,074.32 | baseline |
| fastcond_strong | 1,042,219.59 | -7.9% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 1.36 | 0.05 | 1789.88 | 35.21 |
| fastcond_strong | 2.71 | 0.04 | 1750.09 | 52.96 |
