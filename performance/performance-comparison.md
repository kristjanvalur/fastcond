# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fc | 5,778 | 1.01x |
| linux | gil_test | 4 | fc | 5,791 | 1.01x |
| linux | gil_test | 4 | native | 5,743 | 1.00x |
| linux | gil_test | 4 | native | 5,749 | 1.00x |
| linux | qtest | 4 | fc | 687,703 | 1.11x |
| linux | qtest | 4 | fc | 764,189 | 1.23x |
| linux | qtest | 4 | native | 619,927 | 1.00x |
| linux | qtest | 4 | native | 635,241 | 1.02x |
| linux | strongtest | 1 | fc | 1,401,197 | 1.26x |
| linux | strongtest | 1 | fc | 1,380,941 | 1.25x |
| linux | strongtest | 1 | native | 1,108,778 | 1.00x |
| linux | strongtest | 1 | native | 1,042,290 | 0.94x |
| macos | gil_test | 4 | fc | 1,406 | 0.88x |
| macos | gil_test | 4 | native | 1,595 | 1.00x |
| macos | qtest | 4 | fc | 140,177 | 1.13x |
| macos | qtest | 4 | native | 123,798 | 1.00x |
| macos | strongtest | 1 | fc | 1,645,394 | 10.91x |
| macos | strongtest | 1 | native | 150,797 | 1.00x |
| windows | gil_test | 4 | fc | 2,066 | 1.03x |
| windows | gil_test | 4 | native | 2,014 | 1.00x |
| windows | qtest | 4 | fc | 794,469 | 0.98x |
| windows | qtest | 4 | native | 811,356 | 1.00x |
| windows | strongtest | 1 | fc | 2,984,026 | 0.99x |
| windows | strongtest | 1 | native | 3,002,456 | 1.00x |