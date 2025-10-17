# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (10K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 588,887.57 | +18.4% |
| fastcond_weak | 558,519.53 | +12.3% |
| pthread | 497,424.86 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 9.39 | 0.06 | 101.45 | 12.16 |
| fastcond_strong | 8.88 | 0.04 | 75.34 | 9.74 |
| fastcond_weak | 9.23 | 0.10 | 67.88 | 9.63 |

## STRONGTEST
**Configuration:** Strong semantics test (10K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs pthread |
|---|---:|---:|
| fastcond_strong | 1,312,432.41 | +66.4% |
| pthread | 788,696.78 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| pthread | 2.85 | 0.07 | 997.53 | 27.20 |
| fastcond_strong | 1.97 | 0.08 | 999.57 | 34.46 |
