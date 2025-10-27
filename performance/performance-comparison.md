# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,792 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,809 | N/A |
| linux | gil_test | 4 | native_gil | 5,748 | N/A |
| linux | gil_test | 4 | native_gil | 6,000 | N/A |
| linux | gil_test | 4 | native_gil | 5,782 | N/A |
| linux | gil_test | 4 | native_gil | 5,907 | N/A |
| linux | qtest | 4 | fastcond_cond | 690,385 | 1.15x |
| linux | qtest | 4 | fastcond_cond | 645,994 | 1.08x |
| linux | qtest | 4 | native | 600,458 | 1.00x |
| linux | qtest | 4 | native | 583,924 | 0.97x |
| linux | strongtest | 1 | fastcond_cond | 1,377,452 | 1.17x |
| linux | strongtest | 1 | fastcond_cond | 1,417,103 | 1.21x |
| linux | strongtest | 1 | native | 1,175,935 | 1.00x |
| linux | strongtest | 1 | native | 1,094,487 | 0.93x |
| macos | gil_test | 4 | fastcond_gil | 688 | N/A |
| macos | gil_test | 4 | native_gil | 886 | N/A |
| macos | gil_test | 4 | native_gil | 1,077 | N/A |
| macos | qtest | 4 | fastcond_cond | 736,377 | 1.09x |
| macos | qtest | 4 | native | 677,323 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 2,939,447 | 7.17x |
| macos | strongtest | 1 | native | 409,735 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,079 | N/A |
| windows | gil_test | 4 | native_gil | 2,071 | N/A |
| windows | gil_test | 4 | native_gil | 2,131 | N/A |
| windows | qtest | 4 | fastcond_cond | 653,270 | 0.99x |
| windows | qtest | 4 | native | 663,104 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,386,065 | 1.03x |
| windows | strongtest | 1 | native | 2,327,530 | 1.00x |