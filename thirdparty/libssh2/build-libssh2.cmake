include_guard(GLOBAL)

set(DBUILD_STATIC_LIBS OFF CACHE BOOL "My option" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fineftp-server/libssh2 thirdparty/libssh2 EXCLUDE_FROM_ALL)
add_library(LibSSH2 ALIAS libssh2)