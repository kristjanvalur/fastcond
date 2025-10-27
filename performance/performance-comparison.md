# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,755 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,749 | N/A |
| linux | gil_test | 4 | native_gil | 5,707 | N/A |
| linux | gil_test | 4 | native_gil | 6,018 | N/A |
| linux | gil_test | 4 | native_gil | 5,729 | N/A |
| linux | gil_test | 4 | native_gil | 5,894 | N/A |
| linux | qtest | 4 | fastcond_cond | 799,705 | 1.50x |
| linux | qtest | 4 | fastcond_cond | 635,493 | 1.19x |
| linux | qtest | 4 | native | 532,953 | 1.00x |
| linux | qtest | 4 | native | 601,590 | 1.13x |
| linux | strongtest | 1 | fastcond_cond | 1,436,087 | 1.17x |
| linux | strongtest | 1 | fastcond_cond | 1,400,139 | 1.14x |
| linux | strongtest | 1 | native | 1,225,442 | 1.00x |
| linux | strongtest | 1 | native | 1,295,190 | 1.06x |
| macos | gil_test | 4 | fastcond_gil | 1,121 | N/A |
| macos | gil_test | 4 | native_gil | 1,623 | N/A |
| macos | gil_test | 4 | native_gil | 1,777 | N/A |
| macos | qtest | 4 | fastcond_cond | 516,983 | 0.86x |
| macos | qtest | 4 | native | 600,528 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 641,313 | 2.48x |
| macos | strongtest | 1 | native | 258,144 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,285 | N/A |
| windows | gil_test | 4 | native_gil | 2,285 | N/A |
| windows | gil_test | 4 | native_gil | 2,206 | N/A |
| windows | qtest | 4 | fastcond_cond | 700,187 | 1.02x |
| windows | qtest | 4 | native | 683,751 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,245,223 | 0.86x |
| windows | strongtest | 1 | native | 2,603,082 | 1.00x |