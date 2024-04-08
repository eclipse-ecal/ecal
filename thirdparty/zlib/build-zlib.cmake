include_guard(GLOBAL)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/zlib thirdparty/zlib EXCLUDE_FROM_ALL SYSTEM)
add_library(ZLIB::zlibstatic ALIAS zlibstatic)
add_library(ZLIB::zlib ALIAS zlib)