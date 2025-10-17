# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_weak | 618,294.60 | +40.7% |
| fastcond_strong | 607,738.52 | +38.3% |
| pthread | 439,315.29 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 11.65 | 0.06 | 94.95 | 13.78 |
| fastcond_strong | 8.30 | 0.05 | 95.15 | 9.04 |
| fastcond_weak | 8.00 | 0.06 | 77.27 | 8.97 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,286,464.02 | +21.6% |
| pthread | 1,057,590.23 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.52 | 0.08 | 1446.00 | 32.75 |
| fastcond_strong | 2.09 | 0.10 | 1640.17 | 37.07 |
