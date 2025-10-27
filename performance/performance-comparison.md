# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,764 | N/A |
| linux | gil_test | 4 | fastcond_gil | 4,952 | N/A |
| linux | gil_test | 4 | native_gil | 5,776 | N/A |
| linux | gil_test | 4 | native_gil | 6,028 | N/A |
| linux | gil_test | 4 | native_gil | 5,195 | N/A |
| linux | gil_test | 4 | native_gil | 5,523 | N/A |
| linux | qtest | 4 | fastcond_cond | 709,947 | 1.17x |
| linux | qtest | 4 | fastcond_cond | 692,008 | 1.14x |
| linux | qtest | 4 | native | 605,364 | 1.00x |
| linux | qtest | 4 | native | 567,152 | 0.94x |
| linux | strongtest | 1 | fastcond_cond | 1,364,243 | 1.49x |
| linux | strongtest | 1 | fastcond_cond | 1,397,281 | 1.52x |
| linux | strongtest | 1 | native | 916,414 | 1.00x |
| linux | strongtest | 1 | native | 1,256,048 | 1.37x |
| macos | gil_test | 4 | fastcond_gil | 1,335 | N/A |
| macos | gil_test | 4 | native_gil | 1,478 | N/A |
| macos | gil_test | 4 | native_gil | 1,394 | N/A |
| macos | qtest | 4 | fastcond_cond | 239,733 | 1.16x |
| macos | qtest | 4 | native | 206,714 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 271,828 | 1.45x |
| macos | strongtest | 1 | native | 187,182 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,183 | N/A |
| windows | gil_test | 4 | native_gil | 2,207 | N/A |
| windows | gil_test | 4 | native_gil | 2,210 | N/A |
| windows | qtest | 4 | fastcond_cond | 509,427 | 0.99x |
| windows | qtest | 4 | native | 516,980 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 1,393,845 | 0.45x |
| windows | strongtest | 1 | native | 3,096,359 | 1.00x |