add_subdirectory(thirdparty/zlib/zlib)
add_library(ZLIB::zlibstatic ALIAS zlibstatic)
add_library(ZLIB::zlib ALIAS zlib)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)