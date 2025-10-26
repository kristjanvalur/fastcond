# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,744 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,716 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,993 | N/A |
| linux | qtest | 4 | native | 403,182 | 1.00x |
| linux | qtest | 4 | native | 592,498 | 1.47x |
| linux | qtest | 4 | native | 518,650 | 1.29x |
| linux | strongtest | 1 | native | 1,088,503 | 1.00x |
| linux | strongtest | 1 | native | 1,314,821 | 1.21x |