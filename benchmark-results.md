# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 717,788.64 | +3.2% |
| native | 695,620.50 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 8.05 | 0.05 | 133.19 | 9.73 |
| fastcond_strong | 7.32 | 0.04 | 857.74 | 9.40 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 1,055,789.72 | +0.7% |
| native | 1,048,527.73 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.54 | 0.05 | 8011.50 | 45.00 |
| fastcond_strong | 2.51 | 0.05 | 3911.51 | 44.11 |
