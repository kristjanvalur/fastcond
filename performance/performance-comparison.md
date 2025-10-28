# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,792 | 1.01x |
| linux | gil_test | 4 | fc | 5,779 | 1.00x |
| linux | gil_test | 4 | native | 5,760 | 1.00x |
| linux | gil_test | 4 | native | 5,753 | 1.00x |
| linux | qtest | 4 | fc | 800,972 | 1.28x |
| linux | qtest | 4 | fc | 796,380 | 1.27x |
| linux | qtest | 4 | native | 625,378 | 1.00x |
| linux | qtest | 4 | native | 634,306 | 1.01x |
| linux | strongtest | 1 | fc | 1,429,275 | 1.38x |
| linux | strongtest | 1 | fc | 1,429,264 | 1.38x |
| linux | strongtest | 1 | native | 1,035,730 | 1.00x |
| linux | strongtest | 1 | native | 1,163,544 | 1.12x |
| macos | gil_test | 4 | fc | 1,397 | 0.79x |
| macos | gil_test | 4 | native | 1,762 | 1.00x |
| macos | qtest | 4 | fc | 694,320 | 1.27x |
| macos | qtest | 4 | native | 547,195 | 1.00x |
| macos | strongtest | 1 | fc | 3,376,342 | 15.52x |
| macos | strongtest | 1 | native | 217,522 | 1.00x |