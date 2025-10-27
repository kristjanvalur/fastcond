# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,743 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,748 | N/A |
| linux | gil_test | 4 | native_gil | 5,776 | N/A |
| linux | gil_test | 4 | native_gil | 6,004 | N/A |
| linux | gil_test | 4 | native_gil | 5,707 | N/A |
| linux | gil_test | 4 | native_gil | 5,975 | N/A |
| linux | qtest | 4 | fastcond_cond | 716,232 | 1.60x |
| linux | qtest | 4 | fastcond_cond | 739,390 | 1.65x |
| linux | qtest | 4 | native | 448,316 | 1.00x |
| linux | qtest | 4 | native | 636,044 | 1.42x |
| linux | strongtest | 1 | fastcond_cond | 1,339,290 | 1.13x |
| linux | strongtest | 1 | fastcond_cond | 1,427,117 | 1.21x |
| linux | strongtest | 1 | native | 1,181,437 | 1.00x |
| linux | strongtest | 1 | native | 1,411,521 | 1.19x |
| macos | gil_test | 4 | fastcond_gil | 502 | N/A |
| macos | gil_test | 4 | native_gil | 1,376 | N/A |
| macos | gil_test | 4 | native_gil | 1,399 | N/A |
| macos | qtest | 4 | fastcond_cond | 253,582 | 2.02x |
| macos | qtest | 4 | native | 125,712 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 269,070 | 1.19x |
| macos | strongtest | 1 | native | 226,147 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,262 | N/A |
| windows | gil_test | 4 | native_gil | 2,127 | N/A |
| windows | gil_test | 4 | native_gil | 2,052 | N/A |
| windows | qtest | 4 | fastcond_cond | 652,469 | 1.11x |
| windows | qtest | 4 | native | 585,847 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,517,116 | 1.00x |
| windows | strongtest | 1 | native | 2,510,166 | 1.00x |