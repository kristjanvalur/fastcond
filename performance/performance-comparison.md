# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,780 | 1.01x |
| linux | gil_test | 4 | fc | 5,782 | 1.01x |
| linux | gil_test | 4 | native | 5,733 | 1.00x |
| linux | gil_test | 4 | native | 5,745 | 1.00x |
| linux | qtest | 4 | fc | 753,126 | 1.34x |
| linux | qtest | 4 | fc | 768,807 | 1.37x |
| linux | qtest | 4 | native | 562,368 | 1.00x |
| linux | qtest | 4 | native | 566,005 | 1.01x |
| linux | strongtest | 1 | fc | 1,404,743 | 1.19x |
| linux | strongtest | 1 | fc | 1,379,149 | 1.17x |
| linux | strongtest | 1 | native | 1,178,451 | 1.00x |
| linux | strongtest | 1 | native | 1,261,448 | 1.07x |
| macos | gil_test | 4 | fc | 1,316 | 0.63x |
| macos | gil_test | 4 | native | 2,102 | 1.00x |
| macos | qtest | 4 | fc | 32,842 | 1.16x |
| macos | qtest | 4 | native | 28,335 | 1.00x |
| macos | strongtest | 1 | fc | 8,074 | 0.61x |
| macos | strongtest | 1 | native | 13,310 | 1.00x |
| windows | gil_test | 4 | fc | 2,134 | 1.03x |
| windows | gil_test | 4 | native | 2,070 | 1.00x |
| windows | qtest | 4 | fc | 754,863 | 1.03x |
| windows | qtest | 4 | native | 731,078 | 1.00x |
| windows | strongtest | 1 | fc | 2,775,553 | 0.98x |
| windows | strongtest | 1 | native | 2,823,476 | 1.00x |