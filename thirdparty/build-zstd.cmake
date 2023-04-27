set(ZSTD_BUILD_STATIC ON CACHE BOOL "Build ZSTD Static" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Build ZSTD Programs" FORCE)
add_subdirectory(thirdparty/zstd/build/cmake)

# this is the spelling of the targets from Conan Center Index
add_library(zstd::libzstd_static ALIAS libzstd_static)

#find_package(zstd REQUIRED)