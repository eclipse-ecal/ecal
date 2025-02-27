include_guard(GLOBAL)

# Build as static library
set(TCP_PUBSUB_BUILD_SAMPLES       OFF)
set(TCP_PUBSUB_BUILD_ECAL_SAMPLES  OFF)
set(TCP_PUBSUB_USE_BUILTIN_ASIO    OFF)
set(TCP_PUBSUB_USE_BUILTIN_RECYCLE OFF)
set(TCP_PUBSUB_BUILD_TESTS         OFF)
set(TCP_PUBSUB_LIBRARY_TYPE        STATIC)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tcp_pubsub/tcp_pubsub thirdparty/tcp_pubsub EXCLUDE_FROM_ALL SYSTEM)

set_property(TARGET tcp_pubsub PROPERTY FOLDER thirdparty/tcp_pubsub)
