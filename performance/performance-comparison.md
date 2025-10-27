# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,759 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,680 | N/A |
| linux | gil_test | 4 | native_gil | 5,773 | N/A |
| linux | gil_test | 4 | native_gil | 6,008 | N/A |
| linux | gil_test | 4 | native_gil | 5,713 | N/A |
| linux | gil_test | 4 | native_gil | 5,941 | N/A |
| linux | qtest | 4 | fastcond_cond | 577,648 | 1.20x |
| linux | qtest | 4 | fastcond_cond | 746,698 | 1.56x |
| linux | qtest | 4 | native | 479,982 | 1.00x |
| linux | qtest | 4 | native | 644,237 | 1.34x |
| linux | strongtest | 1 | fastcond_cond | 1,399,407 | 1.19x |
| linux | strongtest | 1 | fastcond_cond | 1,440,991 | 1.22x |
| linux | strongtest | 1 | native | 1,179,675 | 1.00x |
| linux | strongtest | 1 | native | 1,294,608 | 1.10x |
| macos | gil_test | 4 | fastcond_gil | 893 | N/A |
| macos | gil_test | 4 | native_gil | 1,883 | N/A |
| macos | gil_test | 4 | native_gil | 1,708 | N/A |
| macos | qtest | 4 | fastcond_cond | 429,904 | 1.66x |
| macos | qtest | 4 | native | 259,041 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 2,188,184 | 6.58x |
| macos | strongtest | 1 | native | 332,392 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,227 | N/A |
| windows | gil_test | 4 | native_gil | 2,123 | N/A |
| windows | gil_test | 4 | native_gil | 2,276 | N/A |
| windows | qtest | 4 | fastcond_cond | 654,999 | 0.96x |
| windows | qtest | 4 | native | 680,004 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,143,209 | 0.83x |
| windows | strongtest | 1 | native | 2,578,715 | 1.00x |