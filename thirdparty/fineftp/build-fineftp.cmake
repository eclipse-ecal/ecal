include_guard(GLOBAL)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fineftp-server/fineftp-server" "${eCAL_BINARY_DIR}/thirdparty/fineftp" EXCLUDE_FROM_ALL SYSTEM)
set_property(TARGET server PROPERTY FOLDER thirdparty/fineftp)
