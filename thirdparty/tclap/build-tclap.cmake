add_library(tclap INTERFACE)
target_include_directories(tclap INTERFACE ${CMAKE_CURRENT_LIST_DIR}/tclap/include)

add_library(tclap::tclap ALIAS tclap)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)