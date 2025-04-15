# Some tinyxml2 versions install a CMake config
find_package(tinyxml2 CONFIG)

# Otherwise fallback to using the pkgconfig spec
if(NOT tinyxml2_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(tinyxml2 IMPORTED_TARGET GLOBAL tinyxml2)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(tinyxml2 REQUIRED_VARS
    tinyxml2_INCLUDE_DIRS
    tinyxml2_LIBRARIES
    VERSION_VAR tinyxml2_VERSION
  )

  # Add a compatability alias
  if(tinyxml2_FOUND AND NOT TARGET tinyxml2::tinyxml2)
    add_library(tinyxml2::tinyxml2 ALIAS PkgConfig::tinyxml2)
  endif()
endif()
