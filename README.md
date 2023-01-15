# Microbenchmarking sqlite3 batch inserts

Consider using `vcpkg` to install dependencies

## Macbook air m2:

| Benchmark                              | Time       | CPU        | Iterations |
|----------------------------------------|------------|------------|------------|
| BM_singleAddInstruction_noPrepare      | 1229701 ns | 1223272 ns | 518        |
| BM_multiAddInstruction_noPrepare       | 5561057 ns | 5435578 ns | 128        |
| BM_singleAddInstruction_preparedInsert | 581673 ns  | 571932 ns  | 1221       |
| BM_multiAddInstruction_prepared8        | 2700249 ns | 2571682 ns | 274        |

## gcc + stdlib 9980HK 

| Benchmark                              | Time       | CPU        | Iterations |
|----------------------------------------|------------|------------|------------|
| BM_singleAddInstruction_noPrepare      | 1650014 ns | 1643448 ns | 456        |
| BM_singleAddInstruction_preparedInsert | 476670 ns  | 474903 ns  | 1477       |
| BM_multiAddInstruction_noPrepare       | 1011376 ns | 1008385 ns | 697        |
| BM_multiAddInstruction_prepared8       | 405394 ns  | 403389 ns  | 1735       |
| BM_multiAddInstruction_prepared16      | 374910 ns  | 373568 ns  | 1874       |
| BM_multiAddInstruction_prepared32      | 379287 ns  | 377593 ns  | 1849       |

Prepared statements + batch size = 32 gives about 25% speedup