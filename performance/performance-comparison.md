# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,724 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,788 | N/A |
| linux | gil_test | 4 | native_gil | 5,681 | N/A |
| linux | gil_test | 4 | native_gil | 5,961 | N/A |
| linux | gil_test | 4 | native_gil | 5,765 | N/A |
| linux | gil_test | 4 | native_gil | 6,002 | N/A |
| linux | qtest | 4 | fastcond_cond | 562,022 | 0.85x |
| linux | qtest | 4 | fastcond_cond | 636,702 | 0.96x |
| linux | qtest | 4 | native | 661,881 | 1.00x |
| linux | qtest | 4 | native | 417,151 | 0.63x |
| linux | strongtest | 1 | fastcond_cond | 1,412,986 | 1.12x |
| linux | strongtest | 1 | fastcond_cond | 1,389,834 | 1.11x |
| linux | strongtest | 1 | native | 1,256,980 | 1.00x |
| linux | strongtest | 1 | native | 1,200,717 | 0.96x |
| macos | gil_test | 4 | fastcond_gil | 848 | N/A |
| macos | gil_test | 4 | native_gil | 1,868 | N/A |
| macos | gil_test | 4 | native_gil | 1,654 | N/A |
| macos | qtest | 4 | fastcond_cond | 223,065 | 0.59x |
| macos | qtest | 4 | native | 380,055 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 196,498 | 1.27x |
| macos | strongtest | 1 | native | 154,392 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 1,964 | N/A |
| windows | gil_test | 4 | native_gil | 2,225 | N/A |
| windows | gil_test | 4 | native_gil | 2,015 | N/A |
| windows | qtest | 4 | fastcond_cond | 457,804 | 0.75x |
| windows | qtest | 4 | native | 611,860 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,036,784 | 0.86x |
| windows | strongtest | 1 | native | 2,370,792 | 1.00x |