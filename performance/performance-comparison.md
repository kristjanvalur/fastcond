# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,759 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,593 | N/A |
| linux | gil_test | 4 | native_gil | 5,757 | N/A |
| linux | gil_test | 4 | native_gil | 6,015 | N/A |
| linux | gil_test | 4 | native_gil | 5,745 | N/A |
| linux | gil_test | 4 | native_gil | 6,000 | N/A |
| linux | qtest | 4 | fastcond_cond | 580,030 | 1.24x |
| linux | qtest | 4 | fastcond_cond | 484,395 | 1.04x |
| linux | qtest | 4 | fastcond_wcond | 581,726 | 1.25x |
| linux | qtest | 4 | fastcond_wcond | 666,448 | 1.43x |
| linux | qtest | 4 | native | 466,113 | 1.00x |
| linux | qtest | 4 | native | 388,655 | 0.83x |
| linux | strongtest | 1 | fastcond_cond | 1,421,317 | 1.27x |
| linux | strongtest | 1 | fastcond_cond | 1,417,407 | 1.27x |
| linux | strongtest | 1 | native | 1,120,405 | 1.00x |
| linux | strongtest | 1 | native | 1,388,094 | 1.24x |
| macos | gil_test | 4 | fastcond_gil | 1,188 | N/A |
| macos | gil_test | 4 | native_gil | 1,383 | N/A |
| macos | gil_test | 4 | native_gil | 1,454 | N/A |
| macos | qtest | 4 | fastcond_cond | 590,005 | 1.27x |
| macos | qtest | 4 | fastcond_wcond | 635,688 | 1.37x |
| macos | qtest | 4 | native | 464,188 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 3,034,901 | 6.17x |
| macos | strongtest | 1 | native | 491,908 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,214 | N/A |
| windows | gil_test | 4 | native_gil | 2,163 | N/A |
| windows | gil_test | 4 | native_gil | 2,130 | N/A |
| windows | qtest | 4 | fastcond_cond | 637,674 | 0.91x |
| windows | qtest | 4 | fastcond_wcond | 638,036 | 0.91x |
| windows | qtest | 4 | native | 699,893 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,007,025 | 1.07x |
| windows | strongtest | 1 | native | 1,873,887 | 1.00x |