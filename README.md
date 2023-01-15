# Microbenchmarking sqlite3 batch inserts

Consider using `vcpkg` to install dependencies

## Macbook air m2:

| Benchmark                              | Time       | CPU        | Iterations |
|----------------------------------------|------------|------------|------------|
| BM_singleAddInstruction_noPrepare      | 1205605 ns | 1194327 ns | 523        |
| BM_singleAddInstruction_preparedInsert | 575347 ns  | 562414 ns  | 1255       |
| BM_multiAddInstruction_noPrepare       | 870137 ns  | 862669 ns  | 801        |
| BM_multiAddInstruction_prepared8       | 528379 ns  | 519480 ns  | 1343       |
| BM_multiAddInstruction_prepared16      | 507758 ns  | 500504 ns  | 1000       |
| BM_multiAddInstruction_prepared32      | 502402 ns  | 495075 ns  | 1417       |

Prepared statements + batch size = 32 gives about 15% speedup

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