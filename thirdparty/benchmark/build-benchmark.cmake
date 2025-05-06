set(CMAKE_BUILD_TYPE Release)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "My Option" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "My Option" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/benchmark" "${eCAL_BINARY_DIR}/thirdparty/benchmark" EXCLUDE_FROM_ALL SYSTEM)

if(NOT TARGET benchmark::benchmark)
  add_library(benchmark::benchmark)
endif()
if(NOT TARGET benchmark::benchmark_main)
  add_library(benchmark::benchmark_main)
endif()

set_property(TARGET benchmark PROPERTY FOLDER thirdparty/benchmark)