# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,756 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,739 | N/A |
| linux | gil_test | 4 | fastcond_gil | 6,021 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,582 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,705 | N/A |
| linux | gil_test | 4 | fastcond_gil | 6,025 | N/A |
| linux | qtest | 4 | native | 401,764 | 1.00x |
| linux | qtest | 4 | native | 795,442 | 1.98x |
| linux | qtest | 4 | native | 497,116 | 1.24x |
| linux | qtest | 4 | native | 453,871 | 1.13x |
| linux | qtest | 4 | native | 489,085 | 1.22x |
| linux | qtest | 4 | native | 569,552 | 1.42x |
| linux | strongtest | 1 | native | 1,021,196 | 1.00x |
| linux | strongtest | 1 | native | 612,659 | 0.60x |
| linux | strongtest | 1 | native | 1,238,725 | 1.21x |
| linux | strongtest | 1 | native | 1,339,329 | 1.31x |
| macos | gil_test | 4 | fastcond_gil | 767 | N/A |
| macos | gil_test | 4 | fastcond_gil | 728 | N/A |
| macos | gil_test | 4 | fastcond_gil | 745 | N/A |
| macos | qtest | 4 | native | 579,240 | 1.00x |
| macos | qtest | 4 | native | 634,357 | 1.10x |
| macos | qtest | 4 | native | 635,284 | 1.10x |
| macos | strongtest | 1 | native | 401,913 | 1.00x |
| macos | strongtest | 1 | native | 355,758 | 0.89x |
| windows | gil_test | 4 | fastcond_gil | 2,113 | N/A |
| windows | gil_test | 4 | fastcond_gil | 2,129 | N/A |
| windows | gil_test | 4 | fastcond_gil | 2,058 | N/A |
| windows | qtest | 4 | native | 694,440 | 1.00x |
| windows | qtest | 4 | native | 694,117 | 1.00x |
| windows | qtest | 4 | native | 568,553 | 0.82x |
| windows | strongtest | 1 | native | 2,573,605 | 1.00x |
| windows | strongtest | 1 | native | 2,534,790 | 0.98x |