# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,804 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,804 | N/A |
| linux | gil_test | 4 | native_gil | 5,723 | N/A |
| linux | gil_test | 4 | native_gil | 5,992 | N/A |
| linux | gil_test | 4 | native_gil | 5,762 | N/A |
| linux | gil_test | 4 | native_gil | 5,960 | N/A |
| linux | qtest | 4 | fastcond_cond | 602,811 | 1.28x |
| linux | qtest | 4 | fastcond_cond | 731,410 | 1.56x |
| linux | qtest | 4 | native | 470,185 | 1.00x |
| linux | qtest | 4 | native | 600,951 | 1.28x |
| linux | strongtest | 1 | fastcond_cond | 1,390,944 | 1.38x |
| linux | strongtest | 1 | fastcond_cond | 1,230,774 | 1.22x |
| linux | strongtest | 1 | native | 1,006,214 | 1.00x |
| linux | strongtest | 1 | native | 1,209,235 | 1.20x |
| macos | gil_test | 4 | fastcond_gil | 633 | N/A |
| macos | gil_test | 4 | native_gil | 996 | N/A |
| macos | gil_test | 4 | native_gil | 1,014 | N/A |
| macos | qtest | 4 | fastcond_cond | 284,422 | 0.99x |
| macos | qtest | 4 | native | 287,175 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 273,261 | 4.69x |
| macos | strongtest | 1 | native | 58,242 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,356 | N/A |
| windows | gil_test | 4 | native_gil | 2,134 | N/A |
| windows | gil_test | 4 | native_gil | 2,149 | N/A |
| windows | qtest | 4 | fastcond_cond | 660,319 | 1.06x |
| windows | qtest | 4 | native | 621,326 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,481,697 | 1.02x |
| windows | strongtest | 1 | native | 2,426,066 | 1.00x |