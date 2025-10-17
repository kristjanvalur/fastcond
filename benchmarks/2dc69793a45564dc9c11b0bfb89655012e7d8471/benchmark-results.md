# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 583,287.74 | +19.4% |
| fastcond_weak | 581,834.94 | +19.1% |
| pthread | 488,355.03 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 9.79 | 0.06 | 96.39 | 12.04 |
| fastcond_strong | 8.60 | 0.06 | 67.78 | 9.90 |
| fastcond_weak | 8.77 | 0.05 | 78.42 | 9.97 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,319,922.92 | +11.6% |
| pthread | 1,183,138.34 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 1.77 | 0.09 | 1242.02 | 28.14 |
| fastcond_strong | 1.10 | 0.07 | 1007.63 | 16.65 |
