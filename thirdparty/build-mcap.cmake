find_package(zstd REQUIRED)
find_package(lz4 REQUIRED)

add_library(mcap INTERFACE)
add_library(mcap::mcap ALIAS mcap)

target_include_directories(mcap INTERFACE ${CMAKE_CURRENT_LIST_DIR}/mcap/cpp/mcap/include)
target_compile_features(mcap INTERFACE cxx_std_17)
target_link_libraries(mcap INTERFACE zstd::libzstd_static LZ4::lz4_static)

# What about install and so on. We will have to see :(
# find_package(mcap REQUIRED)