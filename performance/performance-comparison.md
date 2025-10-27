# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,713 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,718 | N/A |
| linux | gil_test | 4 | native_gil | 5,741 | N/A |
| linux | gil_test | 4 | native_gil | 5,977 | N/A |
| linux | gil_test | 4 | native_gil | 5,614 | N/A |
| linux | gil_test | 4 | native_gil | 5,896 | N/A |
| linux | qtest | 4 | fastcond_cond | 709,480 | 1.29x |
| linux | qtest | 4 | fastcond_cond | 539,546 | 0.98x |
| linux | qtest | 4 | native | 548,279 | 1.00x |
| linux | qtest | 4 | native | 504,855 | 0.92x |
| linux | strongtest | 1 | fastcond_cond | 1,396,063 | 1.12x |
| linux | strongtest | 1 | fastcond_cond | 1,396,840 | 1.12x |
| linux | strongtest | 1 | native | 1,245,275 | 1.00x |
| linux | strongtest | 1 | native | 1,125,844 | 0.90x |
| macos | gil_test | 4 | fastcond_gil | 714 | N/A |
| macos | gil_test | 4 | native_gil | 1,579 | N/A |
| macos | gil_test | 4 | native_gil | 1,693 | N/A |
| macos | qtest | 4 | fastcond_cond | 111,168 | 0.25x |
| macos | qtest | 4 | native | 451,916 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 596,801 | 2.63x |
| macos | strongtest | 1 | native | 226,660 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,133 | N/A |
| windows | gil_test | 4 | native_gil | 2,212 | N/A |
| windows | gil_test | 4 | native_gil | 2,072 | N/A |
| windows | qtest | 4 | fastcond_cond | 643,811 | 0.97x |
| windows | qtest | 4 | native | 666,791 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,317,658 | 1.06x |
| windows | strongtest | 1 | native | 2,177,890 | 1.00x |