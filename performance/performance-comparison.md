# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,704 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,676 | N/A |
| linux | gil_test | 4 | native_gil | 5,734 | N/A |
| linux | gil_test | 4 | native_gil | 6,018 | N/A |
| linux | gil_test | 4 | native_gil | 5,720 | N/A |
| linux | gil_test | 4 | native_gil | 5,953 | N/A |
| linux | qtest | 4 | fastcond_cond | 561,592 | 0.91x |
| linux | qtest | 4 | fastcond_cond | 688,574 | 1.11x |
| linux | qtest | 4 | native | 619,676 | 1.00x |
| linux | qtest | 4 | native | 593,275 | 0.96x |
| linux | strongtest | 1 | fastcond_cond | 1,444,792 | 1.31x |
| linux | strongtest | 1 | fastcond_cond | 1,365,257 | 1.23x |
| linux | strongtest | 1 | native | 1,106,575 | 1.00x |
| linux | strongtest | 1 | native | 1,211,726 | 1.10x |
| macos | gil_test | 4 | fastcond_gil | 1,016 | N/A |
| macos | gil_test | 4 | native_gil | 1,358 | N/A |
| macos | gil_test | 4 | native_gil | 1,458 | N/A |
| macos | qtest | 4 | fastcond_cond | 644,745 | 0.97x |
| macos | qtest | 4 | native | 663,218 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,608,805 | 9.17x |
| macos | strongtest | 1 | native | 393,716 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,477 | N/A |
| windows | gil_test | 4 | native_gil | 2,169 | N/A |
| windows | gil_test | 4 | native_gil | 2,282 | N/A |
| windows | qtest | 4 | fastcond_cond | 655,291 | 0.96x |
| windows | qtest | 4 | native | 684,247 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,393,432 | 1.20x |
| windows | strongtest | 1 | native | 2,001,721 | 1.00x |