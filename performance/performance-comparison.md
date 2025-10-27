# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,739 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,787 | N/A |
| linux | gil_test | 4 | native_gil | 5,776 | N/A |
| linux | gil_test | 4 | native_gil | 6,029 | N/A |
| linux | gil_test | 4 | native_gil | 5,770 | N/A |
| linux | gil_test | 4 | native_gil | 5,974 | N/A |
| linux | qtest | 4 | fastcond_cond | 502,086 | 1.13x |
| linux | qtest | 4 | fastcond_cond | 380,646 | 0.85x |
| linux | qtest | 4 | native | 446,230 | 1.00x |
| linux | qtest | 4 | native | 545,260 | 1.22x |
| linux | strongtest | 1 | fastcond_cond | 1,307,571 | 1.04x |
| linux | strongtest | 1 | fastcond_cond | 1,218,522 | 0.97x |
| linux | strongtest | 1 | native | 1,256,937 | 1.00x |
| linux | strongtest | 1 | native | 1,208,921 | 0.96x |
| macos | gil_test | 4 | fastcond_gil | 903 | N/A |
| macos | gil_test | 4 | native_gil | 1,762 | N/A |
| macos | gil_test | 4 | native_gil | 1,791 | N/A |
| macos | qtest | 4 | fastcond_cond | 171,394 | 0.50x |
| macos | qtest | 4 | native | 341,915 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 666,489 | 3.23x |
| macos | strongtest | 1 | native | 206,548 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,107 | N/A |
| windows | gil_test | 4 | native_gil | 2,282 | N/A |
| windows | gil_test | 4 | native_gil | 2,292 | N/A |
| windows | qtest | 4 | fastcond_cond | 647,501 | 0.94x |
| windows | qtest | 4 | native | 689,931 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 1,946,207 | 0.84x |
| windows | strongtest | 1 | native | 2,303,457 | 1.00x |