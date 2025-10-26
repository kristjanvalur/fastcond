# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,786 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,657 | N/A |
| linux | gil_test | 4 | native_gil | 5,765 | N/A |
| linux | gil_test | 4 | native_gil | 6,025 | N/A |
| linux | gil_test | 4 | native_gil | 5,723 | N/A |
| linux | gil_test | 4 | native_gil | 6,040 | N/A |
| linux | qtest | 4 | fastcond_cond | 597,419 | 1.87x |
| linux | qtest | 4 | fastcond_cond | 688,487 | 2.15x |
| linux | qtest | 4 | fastcond_wcond | 467,059 | 1.46x |
| linux | qtest | 4 | fastcond_wcond | 750,044 | 2.35x |
| linux | qtest | 4 | native | 319,592 | 1.00x |
| linux | qtest | 4 | native | 456,801 | 1.43x |
| linux | strongtest | 1 | fastcond_cond | 1,445,818 | 1.08x |
| linux | strongtest | 1 | fastcond_cond | 1,373,751 | 1.03x |
| linux | strongtest | 1 | native | 1,334,962 | 1.00x |
| linux | strongtest | 1 | native | 1,015,615 | 0.76x |
| macos | gil_test | 4 | fastcond_gil | 1,358 | N/A |
| macos | gil_test | 4 | native_gil | 1,659 | N/A |
| macos | gil_test | 4 | native_gil | 1,683 | N/A |
| macos | qtest | 4 | fastcond_cond | 459,158 | 0.80x |
| macos | qtest | 4 | fastcond_wcond | 559,472 | 0.98x |
| macos | qtest | 4 | native | 570,418 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 1,272,588 | 5.36x |
| macos | strongtest | 1 | native | 237,451 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,046 | N/A |
| windows | gil_test | 4 | native_gil | 1,948 | N/A |
| windows | gil_test | 4 | native_gil | 2,146 | N/A |
| windows | qtest | 4 | fastcond_cond | 522,076 | 1.00x |
| windows | qtest | 4 | fastcond_wcond | 526,676 | 1.01x |
| windows | qtest | 4 | native | 523,755 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 1,928,678 | 1.19x |
| windows | strongtest | 1 | native | 1,614,831 | 1.00x |