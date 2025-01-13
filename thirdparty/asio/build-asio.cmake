include_guard(GLOBAL)

include(GNUInstallDirs)
add_library(asio INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(asio INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/asio/asio/include>
)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

add_library(asio::asio ALIAS asio)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)

# We don't want to install asio. However we need to have the commands
# Otherwise CMake will not be happy
install(
  TARGETS asio
  EXPORT asioTargets
)

install(
  EXPORT asioTargets 
  FILE asioTargets.cmake 
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
  NAMESPACE asio::
  COMPONENT asio_dev
)
