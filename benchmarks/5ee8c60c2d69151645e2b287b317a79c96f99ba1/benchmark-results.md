# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 589,608.67 | +14.1% |
| pthread | 516,538.82 | baseline |
| fastcond_weak | 504,467.31 | -2.3% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 9.39 | 0.07 | 75.77 | 10.92 |
| fastcond_strong | 9.42 | 0.05 | 94.07 | 10.07 |
| fastcond_weak | 10.38 | 0.07 | 78.12 | 11.80 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,211,355.05 | +57.0% |
| pthread | 771,571.18 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 3.15 | 0.04 | 803.80 | 22.01 |
| fastcond_strong | 2.13 | 0.05 | 1010.73 | 32.04 |
