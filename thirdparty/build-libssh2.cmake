set(DBUILD_STATIC_LIBS OFF CACHE BOOL "My option" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
add_subdirectory(thirdparty/libssh2)
add_library(LibSSH2 ALIAS libssh2)