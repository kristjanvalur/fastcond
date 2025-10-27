# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,811 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,772 | N/A |
| linux | gil_test | 4 | native_gil | 5,753 | N/A |
| linux | gil_test | 4 | native_gil | 5,988 | N/A |
| linux | gil_test | 4 | native_gil | 5,698 | N/A |
| linux | gil_test | 4 | native_gil | 5,978 | N/A |
| linux | qtest | 4 | fastcond_cond | 690,321 | 1.47x |
| linux | qtest | 4 | fastcond_cond | 605,693 | 1.29x |
| linux | qtest | 4 | native | 469,200 | 1.00x |
| linux | qtest | 4 | native | 487,613 | 1.04x |
| linux | strongtest | 1 | fastcond_cond | 1,454,710 | 1.12x |
| linux | strongtest | 1 | fastcond_cond | 2,294,177 | 1.76x |
| linux | strongtest | 1 | native | 1,300,725 | 1.00x |
| linux | strongtest | 1 | native | 2,253,659 | 1.73x |
| macos | gil_test | 4 | fastcond_gil | 1,066 | N/A |
| macos | gil_test | 4 | native_gil | 1,504 | N/A |
| macos | gil_test | 4 | native_gil | 1,479 | N/A |
| macos | qtest | 4 | fastcond_cond | 557,227 | 1.05x |
| macos | qtest | 4 | native | 531,858 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 535,160 | 1.28x |
| macos | strongtest | 1 | native | 419,428 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,156 | N/A |
| windows | gil_test | 4 | native_gil | 2,294 | N/A |
| windows | gil_test | 4 | native_gil | 2,146 | N/A |
| windows | qtest | 4 | fastcond_cond | 639,889 | 0.93x |
| windows | qtest | 4 | native | 690,737 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,158,009 | 0.93x |
| windows | strongtest | 1 | native | 2,317,282 | 1.00x |