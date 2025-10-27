# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,808 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,751 | N/A |
| linux | gil_test | 4 | native_gil | 5,785 | N/A |
| linux | gil_test | 4 | native_gil | 6,035 | N/A |
| linux | gil_test | 4 | native_gil | 5,752 | N/A |
| linux | gil_test | 4 | native_gil | 5,977 | N/A |
| linux | qtest | 4 | fastcond_cond | 484,194 | 1.08x |
| linux | qtest | 4 | fastcond_cond | 444,849 | 1.00x |
| linux | qtest | 4 | native | 446,763 | 1.00x |
| linux | qtest | 4 | native | 348,059 | 0.78x |
| linux | strongtest | 1 | fastcond_cond | 1,384,180 | 1.12x |
| linux | strongtest | 1 | fastcond_cond | 1,219,031 | 0.98x |
| linux | strongtest | 1 | native | 1,237,895 | 1.00x |
| linux | strongtest | 1 | native | 1,212,451 | 0.98x |
| macos | gil_test | 4 | fastcond_gil | 1,023 | N/A |
| macos | gil_test | 4 | native_gil | 1,736 | N/A |
| macos | gil_test | 4 | native_gil | 1,728 | N/A |
| macos | qtest | 4 | fastcond_cond | 350,398 | 2.64x |
| macos | qtest | 4 | native | 132,515 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 1,418,239 | 3.03x |
| macos | strongtest | 1 | native | 467,902 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,129 | N/A |
| windows | gil_test | 4 | native_gil | 2,196 | N/A |
| windows | gil_test | 4 | native_gil | 2,126 | N/A |
| windows | qtest | 4 | fastcond_cond | 673,786 | 1.09x |
| windows | qtest | 4 | native | 620,036 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,403,904 | 0.93x |
| windows | strongtest | 1 | native | 2,582,378 | 1.00x |