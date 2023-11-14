add_library(asio::asio INTERFACE IMPORTED)
target_include_directories(asio::asio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/asio/asio/include)
target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)