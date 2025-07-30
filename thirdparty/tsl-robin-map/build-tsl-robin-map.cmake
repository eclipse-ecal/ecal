include_guard(GLOBAL)

message(STATUS "building tsl-robin-map")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.25.0")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tsl-robin-map" "${eCAL_BINARY_DIR}/thirdparty/tsl-robin-map" EXCLUDE_FROM_ALL SYSTEM)
else ()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tsl-robin-map" "${eCAL_BINARY_DIR}/thirdparty/tsl-robin-map" EXCLUDE_FROM_ALL)
endif ()