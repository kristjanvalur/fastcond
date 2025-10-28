# Performance Comparison: fastcond vs Native

| Platform | Test | Threads | Variant | Throughput (ops/sec) | Speedup vs Native |
|----------|------|---------|---------|---------------------|-------------------|
| linux | qtest | 4 | fc | 731,820 | 1.08x |
| linux | qtest | 4 | fc | 731,066 | 1.08x |
| linux | qtest | 4 | native | 679,606 | 1.00x |
| linux | qtest | 4 | native | 604,289 | 0.89x |
| linux | strongtest | 1 | fc | 1,449,363 | 1.22x |
| linux | strongtest | 1 | fc | 1,441,221 | 1.22x |
| linux | strongtest | 1 | native | 1,184,968 | 1.00x |
| linux | strongtest | 1 | native | 979,734 | 0.83x |
| macos | qtest | 4 | fc | 87,911 | 0.56x |
| macos | qtest | 4 | native | 157,922 | 1.00x |
| macos | strongtest | 1 | fc | 325,191 | 3.74x |
| macos | strongtest | 1 | native | 87,015 | 1.00x |