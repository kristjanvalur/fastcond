# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_weak | 619,905.46 | +17.6% |
| fastcond_strong | 573,950.67 | +8.9% |
| pthread | 526,921.22 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 9.58 | 0.07 | 74.51 | 10.18 |
| fastcond_strong | 8.47 | 0.04 | 62.76 | 8.30 |
| fastcond_weak | 7.94 | 0.04 | 71.79 | 8.12 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 2,007,387.99 | +17.4% |
| pthread | 1,710,108.55 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 1.18 | 0.04 | 821.94 | 19.64 |
| fastcond_strong | 0.67 | 0.13 | 216.24 | 6.61 |
