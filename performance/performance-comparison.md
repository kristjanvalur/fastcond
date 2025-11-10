# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,805 | 1.01x |
| linux | gil_test | 4 | fc | 5,779 | 1.00x |
| linux | gil_test | 4 | native | 5,767 | 1.00x |
| linux | gil_test | 4 | native | 5,745 | 1.00x |
| linux | qtest | 4 | fc | 727,397 | 1.12x |
| linux | qtest | 4 | fc | 782,187 | 1.21x |
| linux | qtest | 4 | native | 648,271 | 1.00x |
| linux | qtest | 4 | native | 620,465 | 0.96x |
| linux | strongtest | 1 | fc | 1,430,824 | 1.20x |
| linux | strongtest | 1 | fc | 1,452,103 | 1.22x |
| linux | strongtest | 1 | native | 1,187,506 | 1.00x |
| linux | strongtest | 1 | native | 1,192,604 | 1.00x |
| macos | gil_test | 4 | fc | 1,060 | 0.56x |
| macos | gil_test | 4 | native | 1,893 | 1.00x |
| macos | qtest | 4 | fc | 25,950 | 0.92x |
| macos | qtest | 4 | native | 28,323 | 1.00x |
| macos | strongtest | 1 | fc | 21,975 | 1.66x |
| macos | strongtest | 1 | native | 13,225 | 1.00x |
| windows | gil_test | 4 | fc | 2,028 | 0.96x |
| windows | gil_test | 4 | native | 2,102 | 1.00x |
| windows | qtest | 4 | fc | 764,331 | 1.00x |
| windows | qtest | 4 | native | 765,960 | 1.00x |
| windows | strongtest | 1 | fc | 2,890,370 | 0.97x |
| windows | strongtest | 1 | native | 2,974,799 | 1.00x |