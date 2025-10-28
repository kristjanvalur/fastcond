# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | gil_test | 4 | fastcond_gil | 5,707 | N/A |
| linux | gil_test | 4 | fastcond_gil | 5,734 | N/A |
| linux | gil_test | 4 | native_gil | 5,715 | N/A |
| linux | gil_test | 4 | native_gil | 6,000 | N/A |
| linux | gil_test | 4 | native_gil | 5,745 | N/A |
| linux | gil_test | 4 | native_gil | 6,016 | N/A |
| linux | qtest | 4 | fastcond_cond | 670,184 | 1.15x |
| linux | qtest | 4 | fastcond_cond | 686,521 | 1.17x |
| linux | qtest | 4 | native | 584,380 | 1.00x |
| linux | qtest | 4 | native | 517,529 | 0.89x |
| linux | strongtest | 1 | fastcond_cond | 1,377,929 | 1.34x |
| linux | strongtest | 1 | fastcond_cond | 1,384,526 | 1.34x |
| linux | strongtest | 1 | native | 1,031,772 | 1.00x |
| linux | strongtest | 1 | native | 865,351 | 0.84x |
| macos | gil_test | 4 | fastcond_gil | 830 | N/A |
| macos | gil_test | 4 | native_gil | 1,278 | N/A |
| macos | gil_test | 4 | native_gil | 1,433 | N/A |
| macos | qtest | 4 | fastcond_cond | 248,157 | 0.49x |
| macos | qtest | 4 | native | 510,777 | 1.00x |
| macos | strongtest | 1 | fastcond_cond | 870,398 | 4.71x |
| macos | strongtest | 1 | native | 184,891 | 1.00x |
| windows | gil_test | 4 | fastcond_gil | 2,184 | N/A |
| windows | gil_test | 4 | native_gil | 2,363 | N/A |
| windows | gil_test | 4 | native_gil | 2,202 | N/A |
| windows | qtest | 4 | fastcond_cond | 630,100 | 0.94x |
| windows | qtest | 4 | native | 668,758 | 1.00x |
| windows | strongtest | 1 | fastcond_cond | 2,300,808 | 0.91x |
| windows | strongtest | 1 | native | 2,523,214 | 1.00x |