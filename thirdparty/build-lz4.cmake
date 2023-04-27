set(LZ4_BUILD_CLI OFF CACHE BOOL "Build lz4 cli program" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "Build lz4 cli legacy program" FORCE)

add_subdirectory(thirdparty/lz4/build/cmake)

# this is the spelling of the targets from Conan Center Index
add_library(LZ4::lz4_static ALIAS lz4_static)

#find_package(lz4 REQUIRED)