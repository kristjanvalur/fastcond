# Performance Benchmark Results

Generated from: `/home/runner/work/fastcond/fastcond/docs/benchmark-results.json`

## QTEST
**Configuration:** Producer-consumer queue test (400K items, 4 threads, queue size 10)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| fastcond_strong | 682,309.02 | +10.2% |
| native | 619,271.27 | baseline |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 7.99 | 0.04 | 143.38 | 9.80 |
| fastcond_strong | 6.96 | 0.03 | 101.84 | 7.73 |

## STRONGTEST
**Configuration:** Strong semantics test (400K items, queue size 5)
### Throughput Comparison
| Implementation | Throughput (items/sec) | Speedup vs native |
|---|---:|---:|
| native | 1,112,039.80 | baseline |
| fastcond_strong | 1,076,780.96 | -3.2% |

### Average Latency Comparison
| Implementation | Avg Latency (μs) | Min (μs) | Max (μs) | Stdev (μs) |
|---|---:|---:|---:|---:|
| native | 2.33 | 0.05 | 8011.70 | 48.65 |
| fastcond_strong | 2.31 | 0.03 | 16040.07 | 50.14 |
