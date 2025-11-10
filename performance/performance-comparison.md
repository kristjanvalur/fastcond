# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,813 | 1.01x |
| linux | gil_test | 4 | fc | 5,768 | 1.00x |
| linux | gil_test | 4 | native | 5,775 | 1.00x |
| linux | gil_test | 4 | native | 5,740 | 0.99x |
| linux | qtest | 4 | fc | 700,723 | 1.21x |
| linux | qtest | 4 | fc | 759,542 | 1.31x |
| linux | qtest | 4 | native | 578,374 | 1.00x |
| linux | qtest | 4 | native | 664,783 | 1.15x |
| linux | strongtest | 1 | fc | 1,432,002 | 1.24x |
| linux | strongtest | 1 | fc | 1,452,000 | 1.26x |
| linux | strongtest | 1 | native | 1,153,838 | 1.00x |
| linux | strongtest | 1 | native | 1,203,440 | 1.04x |
| macos | gil_test | 4 | fc | 1,226 | 0.70x |
| macos | gil_test | 4 | native | 1,753 | 1.00x |
| macos | qtest | 4 | fc | 60,668 | 1.09x |
| macos | qtest | 4 | native | 55,491 | 1.00x |
| macos | strongtest | 1 | fc | 33,497 | 1.81x |
| macos | strongtest | 1 | native | 18,460 | 1.00x |
| windows | gil_test | 4 | fc | 2,147 | 1.02x |
| windows | gil_test | 4 | native | 2,107 | 1.00x |
| windows | qtest | 4 | fc | 748,256 | 0.99x |
| windows | qtest | 4 | native | 756,606 | 1.00x |
| windows | strongtest | 1 | fc | 2,862,830 | 1.01x |
| windows | strongtest | 1 | native | 2,839,011 | 1.00x |