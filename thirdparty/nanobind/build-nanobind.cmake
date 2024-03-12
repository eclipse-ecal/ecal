message(STATUS "building nanobind")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.25.0")
add_subdirectory(thirdparty/nanobind/nanobind EXCLUDE_FROM_ALL SYSTEM)
else ()
add_subdirectory(thirdparty/nanobind/nanobind EXCLUDE_FROM_ALL)
endif ()


list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)