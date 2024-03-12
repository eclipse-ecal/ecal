include_guard(GLOBAL)

message(STATUS "building nanobind")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.25.0")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nanobind thirdparty/nanobind EXCLUDE_FROM_ALL SYSTEM)
else ()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nanobind thirdparty/nanobind EXCLUDE_FROM_ALL)
endif ()