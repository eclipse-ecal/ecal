add_subdirectory(thirdparty/zlib)
add_library(ZLIB::zlibstatic ALIAS zlibstatic)
add_library(ZLIB::zlib ALIAS zlib)