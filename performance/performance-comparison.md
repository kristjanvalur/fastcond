# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,730 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,845 | N/A |
| linux | gil_test | 4 | native_gil | 5,716 | N/A |
| linux | gil_test | 4 | native_gil | 6,002 | N/A |
| linux | gil_test | 4 | native_gil | 5,815 | N/A |
| linux | gil_test | 4 | native_gil | 6,046 | N/A |
| linux | qtest | 4 | fastcond_cond | 623,504 | 1.13x |
| linux | qtest | 4 | fastcond_cond | 684,197 | 1.24x |
| linux | qtest | 4 | fastcond_wcond | 686,738 | 1.24x |
| linux | qtest | 4 | fastcond_wcond | 654,913 | 1.18x |
| linux | qtest | 4 | native | 553,533 | 1.00x |
| linux | qtest | 4 | native | 536,575 | 0.97x |
| linux | strongtest | 1 | fastcond_cond | 1,392,247 | 1.09x |
| linux | strongtest | 1 | fastcond_cond | 2,495,701 | 1.96x |
| linux | strongtest | 1 | native | 1,273,628 | 1.00x |
| linux | strongtest | 1 | native | 503,117 | 0.40x |
| macos | gil_test | 4 | fastcond_gil | 1,485 | N/A |
| macos | gil_test | 4 | native_gil | 1,699 | N/A |
| macos | gil_test | 4 | native_gil | 1,378 | N/A |
| macos | qtest | 4 | fastcond_cond | 514,642 | 2.44x |
| macos | qtest | 4 | fastcond_wcond | 462,449 | 2.19x |
| macos | qtest | 4 | native | 210,846 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 138,314 | 0.47x |
| macos | strongtest | 1 | native | 291,877 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,303 | N/A |
| windows | gil_test | 4 | native_gil | 1,993 | N/A |
| windows | gil_test | 4 | native_gil | 2,196 | N/A |
| windows | qtest | 4 | fastcond_cond | 678,224 | 1.07x |
| windows | qtest | 4 | fastcond_wcond | 671,578 | 1.06x |
| windows | qtest | 4 | native | 634,860 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,562,985 | 1.08x |
| windows | strongtest | 1 | native | 2,366,416 | 1.00x |