cmake_minimum_required(VERSION 3.24)
project(sqlite_batch_benchmark)

set(CMAKE_CXX_STANDARD 20)

find_package(benchmark CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE benchmark::benchmark benchmark::benchmark_main)
target_link_libraries(main PRIVATE SQLiteCpp)