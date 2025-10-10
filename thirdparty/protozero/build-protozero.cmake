#Unfortunately, protozero currently does not support targets, so we will define them

include_guard(GLOBAL)

include(GNUInstallDirs)
add_library(protozero INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(protozero INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/protozero/include>
)
add_library(protozero::protozero ALIAS protozero)

# We don't want to install protozero. However we need to have the commands
# Otherwise CMake will not be happy
install(
  TARGETS protozero
  EXPORT protozeroTargets
)

install(
  EXPORT protozeroTargets 
  FILE protozeroTargets.cmake 
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
  NAMESPACE protozero::
  COMPONENT protozero_dev
)
