add_subdirectory(thirdparty/tcp_pubsub/tcp_pubsub/tcp_pubsub EXCLUDE_FROM_ALL)
set_property(TARGET tcp_pubsub PROPERTY FOLDER lib/tcp_pubsub)
add_library(tcp_pubsub::tcp_pubsub ALIAS tcp_pubsub)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)