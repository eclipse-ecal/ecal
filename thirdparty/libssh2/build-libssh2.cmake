set(DBUILD_STATIC_LIBS OFF CACHE BOOL "My option" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
add_subdirectory(thirdparty/libssh2/libssh2)
add_library(LibSSH2 ALIAS libssh2)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)