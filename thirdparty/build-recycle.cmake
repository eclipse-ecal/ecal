add_subdirectory(thirdparty/recycle EXCLUDE_FROM_ALL)
add_library(steinwurf::recycle ALIAS recycle)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/recycle-module)