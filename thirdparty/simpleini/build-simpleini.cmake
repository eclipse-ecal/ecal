add_library(simpleini INTERFACE)
target_include_directories(simpleini INTERFACE ${CMAKE_CURRENT_LIST_DIR}/simpleini)

add_library(simpleini::simpleini ALIAS simpleini)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)