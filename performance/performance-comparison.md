# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,724 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,674 | N/A |
| linux | gil_test | 4 | native_gil | 5,725 | N/A |
| linux | gil_test | 4 | native_gil | 5,932 | N/A |
| linux | gil_test | 4 | native_gil | 5,499 | N/A |
| linux | gil_test | 4 | native_gil | 5,917 | N/A |
| linux | qtest | 4 | fastcond_cond | 608,915 | 1.32x |
| linux | qtest | 4 | fastcond_cond | 715,956 | 1.55x |
| linux | qtest | 4 | native | 462,207 | 1.00x |
| linux | qtest | 4 | native | 570,971 | 1.24x |
| linux | strongtest | 1 | fastcond_cond | 1,337,039 | 1.04x |
| linux | strongtest | 1 | fastcond_cond | 1,347,175 | 1.04x |
| linux | strongtest | 1 | native | 1,291,324 | 1.00x |
| linux | strongtest | 1 | native | 1,204,934 | 0.93x |
| macos | gil_test | 4 | fastcond_gil | 760 | N/A |
| macos | gil_test | 4 | native_gil | 762 | N/A |
| macos | gil_test | 4 | native_gil | 744 | N/A |
| macos | qtest | 4 | fastcond_cond | 714,592 | 0.99x |
| macos | qtest | 4 | native | 725,163 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,214,401 | 8.16x |
| macos | strongtest | 1 | native | 393,701 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,254 | N/A |
| windows | gil_test | 4 | native_gil | 2,366 | N/A |
| windows | gil_test | 4 | native_gil | 2,054 | N/A |
| windows | qtest | 4 | fastcond_cond | 660,690 | 0.94x |
| windows | qtest | 4 | native | 699,868 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,524,806 | 0.98x |
| windows | strongtest | 1 | native | 2,565,089 | 1.00x |