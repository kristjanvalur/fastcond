# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,796 | 1.00x |
| linux | gil_test | 4 | fc | 5,781 | 1.00x |
| linux | gil_test | 4 | native | 5,768 | 1.00x |
| linux | gil_test | 4 | native | 5,751 | 1.00x |
| linux | qtest | 4 | fc | 750,957 | 1.17x |
| linux | qtest | 4 | fc | 742,186 | 1.15x |
| linux | qtest | 4 | native | 642,742 | 1.00x |
| linux | qtest | 4 | native | 668,413 | 1.04x |
| linux | strongtest | 1 | fc | 1,427,913 | 1.29x |
| linux | strongtest | 1 | fc | 1,442,135 | 1.30x |
| linux | strongtest | 1 | native | 1,110,468 | 1.00x |
| linux | strongtest | 1 | native | 1,119,817 | 1.01x |
| macos | qtest | 4 | fc | 640,689 | 6.17x |
| macos | qtest | 4 | native | 103,779 | 1.00x |
| macos | strongtest | 1 | fc | 3,284,668 | 7.14x |
| macos | strongtest | 1 | native | 460,009 | 1.00x |