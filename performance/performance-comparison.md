# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,767 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,725 | N/A |
| linux | gil_test | 4 | native_gil | 5,734 | N/A |
| linux | gil_test | 4 | native_gil | 5,989 | N/A |
| linux | gil_test | 4 | native_gil | 5,778 | N/A |
| linux | gil_test | 4 | native_gil | 5,998 | N/A |
| linux | qtest | 4 | fastcond_cond | 769,377 | 1.52x |
| linux | qtest | 4 | fastcond_cond | 698,234 | 1.38x |
| linux | qtest | 4 | native | 504,576 | 1.00x |
| linux | qtest | 4 | native | 490,294 | 0.97x |
| linux | strongtest | 1 | fastcond_cond | 1,414,479 | 1.50x |
| linux | strongtest | 1 | fastcond_cond | 1,422,487 | 1.51x |
| linux | strongtest | 1 | native | 944,183 | 1.00x |
| linux | strongtest | 1 | native | 1,038,375 | 1.10x |
| macos | gil_test | 4 | fastcond_gil | 713 | N/A |
| macos | gil_test | 4 | native_gil | 1,009 | N/A |
| macos | gil_test | 4 | native_gil | 1,008 | N/A |
| macos | qtest | 4 | fastcond_cond | 705,069 | 1.00x |
| macos | qtest | 4 | native | 706,764 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,097,893 | 7.57x |
| macos | strongtest | 1 | native | 408,981 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,280 | N/A |
| windows | gil_test | 4 | native_gil | 2,225 | N/A |
| windows | gil_test | 4 | native_gil | 2,218 | N/A |
| windows | qtest | 4 | fastcond_cond | 622,192 | 0.97x |
| windows | qtest | 4 | native | 640,426 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,418,614 | 1.27x |
| windows | strongtest | 1 | native | 1,906,142 | 1.00x |