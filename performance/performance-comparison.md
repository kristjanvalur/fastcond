# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,680 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,759 | N/A |
| linux | gil_test | 4 | native_gil | 5,765 | N/A |
| linux | gil_test | 4 | native_gil | 6,029 | N/A |
| linux | gil_test | 4 | native_gil | 5,715 | N/A |
| linux | gil_test | 4 | native_gil | 6,014 | N/A |
| linux | qtest | 4 | fastcond_cond | 703,605 | 1.53x |
| linux | qtest | 4 | fastcond_cond | 735,455 | 1.59x |
| linux | qtest | 4 | native | 461,133 | 1.00x |
| linux | qtest | 4 | native | 577,524 | 1.25x |
| linux | strongtest | 1 | fastcond_cond | 1,362,579 | 1.07x |
| linux | strongtest | 1 | fastcond_cond | 1,406,397 | 1.10x |
| linux | strongtest | 1 | native | 1,275,834 | 1.00x |
| linux | strongtest | 1 | native | 938,653 | 0.74x |
| macos | gil_test | 4 | fastcond_gil | 667 | N/A |
| macos | gil_test | 4 | native_gil | 1,342 | N/A |
| macos | gil_test | 4 | native_gil | 966 | N/A |
| macos | qtest | 4 | fastcond_cond | 480,261 | 0.98x |
| macos | qtest | 4 | native | 488,162 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 1,617,599 | 3.20x |
| macos | strongtest | 1 | native | 505,791 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,157 | N/A |
| windows | gil_test | 4 | native_gil | 2,145 | N/A |
| windows | gil_test | 4 | native_gil | 2,296 | N/A |
| windows | qtest | 4 | fastcond_cond | 604,891 | 0.92x |
| windows | qtest | 4 | native | 658,263 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,451,701 | 1.23x |
| windows | strongtest | 1 | native | 1,987,439 | 1.00x |