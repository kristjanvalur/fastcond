# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,621 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,745 | N/A |
| linux | gil_test | 4 | native_gil | 5,700 | N/A |
| linux | gil_test | 4 | native_gil | 5,990 | N/A |
| linux | gil_test | 4 | native_gil | 5,694 | N/A |
| linux | gil_test | 4 | native_gil | 6,000 | N/A |
| linux | qtest | 4 | fastcond_cond | 710,059 | 1.28x |
| linux | qtest | 4 | fastcond_cond | 564,112 | 1.01x |
| linux | qtest | 4 | native | 556,867 | 1.00x |
| linux | qtest | 4 | native | 606,019 | 1.09x |
| linux | strongtest | 1 | fastcond_cond | 1,421,908 | 1.02x |
| linux | strongtest | 1 | fastcond_cond | 1,353,260 | 0.97x |
| linux | strongtest | 1 | native | 1,390,563 | 1.00x |
| linux | strongtest | 1 | native | 1,264,531 | 0.91x |
| macos | gil_test | 4 | fastcond_gil | 933 | N/A |
| macos | gil_test | 4 | native_gil | 1,119 | N/A |
| macos | gil_test | 4 | native_gil | 1,183 | N/A |
| macos | qtest | 4 | fastcond_cond | 710,934 | 1.24x |
| macos | qtest | 4 | native | 573,691 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,092,146 | 8.37x |
| macos | strongtest | 1 | native | 369,344 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,204 | N/A |
| windows | gil_test | 4 | native_gil | 2,077 | N/A |
| windows | gil_test | 4 | native_gil | 2,143 | N/A |
| windows | qtest | 4 | fastcond_cond | 682,524 | 1.05x |
| windows | qtest | 4 | native | 648,454 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,307,231 | 1.22x |
| windows | strongtest | 1 | native | 1,883,665 | 1.00x |