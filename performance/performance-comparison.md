# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,810 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,761 | N/A |
| linux | gil_test | 4 | native_gil | 5,605 | N/A |
| linux | gil_test | 4 | native_gil | 6,035 | N/A |
| linux | gil_test | 4 | native_gil | 5,754 | N/A |
| linux | gil_test | 4 | native_gil | 6,009 | N/A |
| linux | qtest | 4 | fastcond_cond | 700,321 | 1.78x |
| linux | qtest | 4 | fastcond_cond | 373,110 | 0.95x |
| linux | qtest | 4 | native | 392,495 | 1.00x |
| linux | qtest | 4 | native | 519,981 | 1.32x |
| linux | strongtest | 1 | fastcond_cond | 1,371,253 | 1.18x |
| linux | strongtest | 1 | fastcond_cond | 748,602 | 0.64x |
| linux | strongtest | 1 | native | 1,165,711 | 1.00x |
| linux | strongtest | 1 | native | 535,216 | 0.46x |
| macos | gil_test | 4 | fastcond_gil | 819 | N/A |
| macos | gil_test | 4 | native_gil | 960 | N/A |
| macos | gil_test | 4 | native_gil | 948 | N/A |
| macos | qtest | 4 | fastcond_cond | 682,454 | 1.24x |
| macos | qtest | 4 | native | 548,787 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,240,441 | 11.12x |
| macos | strongtest | 1 | native | 291,520 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,071 | N/A |
| windows | gil_test | 4 | native_gil | 2,123 | N/A |
| windows | gil_test | 4 | native_gil | 2,091 | N/A |
| windows | qtest | 4 | fastcond_cond | 653,804 | 1.18x |
| windows | qtest | 4 | native | 551,764 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,522,323 | 1.03x |
| windows | strongtest | 1 | native | 2,458,331 | 1.00x |