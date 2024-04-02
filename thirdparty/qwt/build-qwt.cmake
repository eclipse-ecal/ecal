# If QWT_LIBRARY_TYPE is empty or not set, it will default to "STATIC"
if(NOT DEFINED QWT_LIBRARY_TYPE OR QWT_LIBRARY_TYPE STREQUAL "")
    set(QWT_LIBRARY_TYPE "STATIC")
endif()
add_subdirectory(thirdparty/qwt)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)