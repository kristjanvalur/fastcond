# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,757 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,756 | N/A |
| linux | gil_test | 4 | native_gil | 5,784 | N/A |
| linux | gil_test | 4 | native_gil | 5,976 | N/A |
| linux | gil_test | 4 | native_gil | 5,750 | N/A |
| linux | gil_test | 4 | native_gil | 5,970 | N/A |
| linux | qtest | 4 | fastcond_cond | 753,488 | 1.53x |
| linux | qtest | 4 | fastcond_cond | 736,311 | 1.50x |
| linux | qtest | 4 | fastcond_wcond | 719,372 | 1.46x |
| linux | qtest | 4 | fastcond_wcond | 647,085 | 1.32x |
| linux | qtest | 4 | native | 491,638 | 1.00x |
| linux | qtest | 4 | native | 508,922 | 1.04x |
| linux | strongtest | 1 | fastcond_cond | 1,416,188 | 1.18x |
| linux | strongtest | 1 | fastcond_cond | 1,388,322 | 1.15x |
| linux | strongtest | 1 | native | 1,203,406 | 1.00x |
| linux | strongtest | 1 | native | 1,255,945 | 1.04x |
| macos | gil_test | 4 | fastcond_gil | 1,341 | N/A |
| macos | gil_test | 4 | native_gil | 812 | N/A |
| macos | gil_test | 4 | native_gil | 897 | N/A |
| macos | qtest | 4 | fastcond_cond | 591,751 | 0.96x |
| macos | qtest | 4 | fastcond_wcond | 625,156 | 1.01x |
| macos | qtest | 4 | native | 617,627 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 1,469,940 | 3.74x |
| macos | strongtest | 1 | native | 392,603 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,059 | N/A |
| windows | gil_test | 4 | native_gil | 2,129 | N/A |
| windows | gil_test | 4 | native_gil | 2,287 | N/A |
| windows | qtest | 4 | fastcond_cond | 679,016 | 1.00x |
| windows | qtest | 4 | fastcond_wcond | 478,114 | 0.71x |
| windows | qtest | 4 | native | 677,176 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,496,505 | 1.12x |
| windows | strongtest | 1 | native | 2,224,298 | 1.00x |