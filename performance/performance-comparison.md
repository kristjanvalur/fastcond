# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | qtest | 4 | fc | 750,364 | 1.30x |
| linux | qtest | 4 | fc | 723,853 | 1.25x |
| linux | qtest | 4 | native | 579,066 | 1.00x |
| linux | qtest | 4 | native | 547,834 | 0.95x |
| linux | strongtest | 1 | fc | 1,409,593 | 1.24x |
| linux | strongtest | 1 | fc | 2,115,694 | 1.86x |
| linux | strongtest | 1 | native | 1,137,538 | 1.00x |
| linux | strongtest | 1 | native | 474,797 | 0.42x |
| macos | qtest | 4 | fc | 74,299 | 0.37x |
| macos | qtest | 4 | native | 202,790 | 1.00x |
| macos | strongtest | 1 | fc | 3,011,487 | 7.85x |
| macos | strongtest | 1 | native | 383,399 | 1.00x |