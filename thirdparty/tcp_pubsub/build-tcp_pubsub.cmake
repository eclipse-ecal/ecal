include_guard(GLOBAL)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tcp_pubsub/tcp_pubsub thirdparty/tcp_pubsub EXCLUDE_FROM_ALL SYSTEM)

set_property(TARGET tcp_pubsub PROPERTY FOLDER thirdparty/tcp_pubsub)