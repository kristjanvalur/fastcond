# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,706 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,652 | N/A |
| linux | gil_test | 4 | native_gil | 5,772 | N/A |
| linux | gil_test | 4 | native_gil | 6,044 | N/A |
| linux | gil_test | 4 | native_gil | 5,690 | N/A |
| linux | gil_test | 4 | native_gil | 5,972 | N/A |
| linux | qtest | 4 | fastcond_cond | 761,251 | 1.32x |
| linux | qtest | 4 | fastcond_cond | 741,369 | 1.29x |
| linux | qtest | 4 | native | 575,592 | 1.00x |
| linux | qtest | 4 | native | 319,331 | 0.55x |
| linux | strongtest | 1 | fastcond_cond | 1,307,882 | 0.97x |
| linux | strongtest | 1 | fastcond_cond | 1,327,579 | 0.98x |
| linux | strongtest | 1 | native | 1,348,738 | 1.00x |
| linux | strongtest | 1 | native | 1,124,653 | 0.83x |
| macos | gil_test | 4 | fastcond_gil | 799 | N/A |
| macos | gil_test | 4 | native_gil | 772 | N/A |
| macos | gil_test | 4 | native_gil | 1,158 | N/A |
| macos | qtest | 4 | fastcond_cond | 206,633 | 1.16x |
| macos | qtest | 4 | native | 178,792 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 204,524 | 0.91x |
| macos | strongtest | 1 | native | 225,220 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,128 | N/A |
| windows | gil_test | 4 | native_gil | 2,137 | N/A |
| windows | gil_test | 4 | native_gil | 2,273 | N/A |
| windows | qtest | 4 | fastcond_cond | 474,152 | 0.96x |
| windows | qtest | 4 | native | 494,237 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 1,885,690 | 1.51x |
| windows | strongtest | 1 | native | 1,251,611 | 1.00x |