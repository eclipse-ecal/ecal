find_path(asio_INCLUDE_DIR
  NAMES asio.hpp
  HINTS
    "${CONAN_ASIO_ROOT}/include"
    "${ECAL_PROJECT_ROOT}/thirdparty/asio/asio/include"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(asio_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find asio library")
  set(asio_FOUND FALSE)
else()
  set(asio_FOUND TRUE)
  set(ASIO_INCLUDE_DIR ${asio_INCLUDE_DIR})
endif()

if(asio_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(asio
    REQUIRED_VARS asio_INCLUDE_DIR)

  if(NOT TARGET asio::asio)
    set(asio_INCLUDE_DIRS ${asio_INCLUDE_DIR})
    add_library(asio::asio INTERFACE IMPORTED)
    set_target_properties(asio::asio PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${asio_INCLUDE_DIR}
      INTERFACE_COMPILE_DEFINITIONS ASIO_STANDALONE)
    mark_as_advanced(asio_INCLUDE_DIR)
  endif()
endif()
