find_path(Asio_INCLUDE_DIR
  NAMES asio.hpp
  HINTS
    "${CONAN_ASIO_ROOT}/include"
    "${ECAL_PROJECT_ROOT}/thirdparty/asio/asio/include"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

if(Asio_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find Asio library")
  set(Asio_FOUND FALSE)
else()
  set(Asio_FOUND TRUE)
  set(ASIO_INCLUDE_DIR ${Asio_INCLUDE_DIR})
endif()

if(Asio_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Asio
    REQUIRED_VARS Asio_INCLUDE_DIR)

  if(NOT TARGET asio::asio)
    set(Asio_INCLUDE_DIRS ${Asio_INCLUDE_DIR})
    add_library(asio::asio INTERFACE IMPORTED)
    set_target_properties(asio::asio PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${Asio_INCLUDE_DIR}
      INTERFACE_COMPILE_DEFINITIONS ASIO_STANDALONE)
    mark_as_advanced(Asio_INCLUDE_DIR)
  endif()
endif()
