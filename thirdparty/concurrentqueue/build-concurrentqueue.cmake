include_guard(GLOBAL)

include(GNUInstallDirs)

add_library(concurrentqueue INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(concurrentqueue INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/concurrentqueue>
)

add_library(concurrentqueue::concurrentqueue ALIAS concurrentqueue)

# We don't want to install concurrentqueue. However we need to have the commands
# Otherwise CMake will not be happy
install(
  TARGETS concurrentqueue
  EXPORT concurrentqueueTargets
)

install(
  EXPORT concurrentqueueTargets
  FILE concurrentqueueTargets.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
  NAMESPACE concurrentqueue::
  COMPONENT concurrentqueue_dev
)
