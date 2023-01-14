# Microbenchmarking sqlite3 batch inserts

Consider using `vcpkg` to install dependencies

### Results 

Macbook air m2:

| Benchmark                              | Time       | CPU        | Iterations |
|----------------------------------------|------------|------------|------------|
| BM_singleAddInstruction_noPrepare      | 1229701 ns | 1223272 ns | 518        |
| BM_multiAddInstruction_noPrepare       | 5561057 ns | 5435578 ns | 128        |
| BM_singleAddInstruction_preparedInsert | 581673 ns  | 571932 ns  | 1221       |
| BM_multiAddInstruction_prepared        | 2700249 ns | 2571682 ns | 274        |
