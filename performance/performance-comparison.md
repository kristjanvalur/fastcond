# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,734 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,673 | N/A |
| linux | gil_test | 4 | native_gil | 5,763 | N/A |
| linux | gil_test | 4 | native_gil | 5,951 | N/A |
| linux | gil_test | 4 | native_gil | 5,746 | N/A |
| linux | gil_test | 4 | native_gil | 5,973 | N/A |
| linux | qtest | 4 | fastcond_cond | 561,783 | 0.97x |
| linux | qtest | 4 | fastcond_cond | 655,195 | 1.13x |
| linux | qtest | 4 | native | 577,867 | 1.00x |
| linux | qtest | 4 | native | 546,710 | 0.95x |
| linux | strongtest | 1 | fastcond_cond | 1,202,554 | 0.98x |
| linux | strongtest | 1 | fastcond_cond | 1,433,469 | 1.17x |
| linux | strongtest | 1 | native | 1,223,666 | 1.00x |
| linux | strongtest | 1 | native | 882,525 | 0.72x |
| macos | gil_test | 4 | fastcond_gil | 1,002 | N/A |
| macos | gil_test | 4 | native_gil | 1,504 | N/A |
| macos | gil_test | 4 | native_gil | 1,446 | N/A |
| macos | qtest | 4 | fastcond_cond | 279,681 | 0.45x |
| macos | qtest | 4 | native | 617,513 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 400,818 | 1.16x |
| macos | strongtest | 1 | native | 346,548 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,237 | N/A |
| windows | gil_test | 4 | native_gil | 2,133 | N/A |
| windows | gil_test | 4 | native_gil | 2,203 | N/A |
| windows | qtest | 4 | fastcond_cond | 668,628 | 0.99x |
| windows | qtest | 4 | native | 674,859 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,354,991 | 1.04x |
| windows | strongtest | 1 | native | 2,269,787 | 1.00x |