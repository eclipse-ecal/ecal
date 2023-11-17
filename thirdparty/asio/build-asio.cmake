add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

add_library(asio::asio ALIAS asio)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)