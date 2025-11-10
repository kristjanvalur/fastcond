# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,783 | 1.01x |
| linux | gil_test | 4 | fc | 5,780 | 1.01x |
| linux | gil_test | 4 | native | 5,750 | 1.00x |
| linux | gil_test | 4 | native | 5,748 | 1.00x |
| linux | qtest | 4 | fc | 714,333 | 1.02x |
| linux | qtest | 4 | fc | 782,912 | 1.12x |
| linux | qtest | 4 | native | 697,958 | 1.00x |
| linux | qtest | 4 | native | 605,689 | 0.87x |
| linux | strongtest | 1 | fc | 1,433,361 | 1.21x |
| linux | strongtest | 1 | fc | 1,447,934 | 1.22x |
| linux | strongtest | 1 | native | 1,189,132 | 1.00x |
| linux | strongtest | 1 | native | 1,235,576 | 1.04x |
| macos | gil_test | 4 | fc | 1,187 | 0.98x |
| macos | gil_test | 4 | native | 1,210 | 1.00x |
| macos | qtest | 4 | fc | 106,960 | 1.01x |
| macos | qtest | 4 | native | 106,150 | 1.00x |
| macos | strongtest | 1 | fc | 69,399 | 1.89x |
| macos | strongtest | 1 | native | 36,738 | 1.00x |
| windows | gil_test | 4 | fc | 2,135 | 1.04x |
| windows | gil_test | 4 | native | 2,058 | 1.00x |
| windows | qtest | 4 | fc | 504,390 | 0.64x |
| windows | qtest | 4 | native | 790,727 | 1.00x |
| windows | strongtest | 1 | fc | 2,556,982 | 0.93x |
| windows | strongtest | 1 | native | 2,745,364 | 1.00x |