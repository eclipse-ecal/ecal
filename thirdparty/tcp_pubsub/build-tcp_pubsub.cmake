include_guard(GLOBAL)

# Build as static library
set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tcp_pubsub/tcp_pubsub thirdparty/tcp_pubsub EXCLUDE_FROM_ALL SYSTEM)

# Reset static / shared libs to old value
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD})
unset(BUILD_SHARED_LIBS_OLD)

set_property(TARGET tcp_pubsub PROPERTY FOLDER thirdparty/tcp_pubsub)
