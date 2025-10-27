# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,759 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,763 | N/A |
| linux | gil_test | 4 | native_gil | 5,739 | N/A |
| linux | gil_test | 4 | native_gil | 6,005 | N/A |
| linux | gil_test | 4 | native_gil | 5,784 | N/A |
| linux | gil_test | 4 | native_gil | 6,074 | N/A |
| linux | qtest | 4 | fastcond_cond | 544,138 | 1.22x |
| linux | qtest | 4 | fastcond_cond | 723,349 | 1.62x |
| linux | qtest | 4 | native | 445,419 | 1.00x |
| linux | qtest | 4 | native | 507,433 | 1.14x |
| linux | strongtest | 1 | fastcond_cond | 1,339,170 | 1.14x |
| linux | strongtest | 1 | fastcond_cond | 2,128,792 | 1.81x |
| linux | strongtest | 1 | native | 1,177,378 | 1.00x |
| linux | strongtest | 1 | native | 270,553 | 0.23x |
| macos | gil_test | 4 | fastcond_gil | 754 | N/A |
| macos | gil_test | 4 | native_gil | 733 | N/A |
| macos | gil_test | 4 | native_gil | 734 | N/A |
| macos | qtest | 4 | fastcond_cond | 695,217 | 0.99x |
| macos | qtest | 4 | native | 699,203 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,322,259 | 8.25x |
| macos | strongtest | 1 | native | 402,658 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,293 | N/A |
| windows | gil_test | 4 | native_gil | 2,128 | N/A |
| windows | gil_test | 4 | native_gil | 2,278 | N/A |
| windows | qtest | 4 | fastcond_cond | 636,468 | 1.02x |
| windows | qtest | 4 | native | 624,926 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,455,796 | 0.93x |
| windows | strongtest | 1 | native | 2,634,977 | 1.00x |