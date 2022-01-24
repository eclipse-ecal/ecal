find_path(Asio_INCLUDE_DIR
  NAMES asio.hpp
  PATHS
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/asio/asio/include
  include
)

if(Asio_INCLUDE_DIR-NOTFOUND)
  message(FATAL_ERROR "Could not find Asio library")
  set(Asio_FOUND False)
else()
  set(Asio_FOUND True)
  # Add workaround for stupid eprosima_find_package()
  set(ASIO_INCLUDE_DIR ${Asio_INCLUDE_DIR})
endif()

if(Asio_FOUND)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Asio
    REQUIRED_VARS Asio_INCLUDE_DIR
  )

  if(NOT TARGET asio::asio)
    set(Asio_INCLUDE_DIRS ${Asio_INCLUDE_DIR})

    add_library(asio::asio INTERFACE IMPORTED)
    set_target_properties(asio::asio PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${Asio_INCLUDE_DIR}
      INTERFACE_COMPILE_DEFINITIONS ASIO_STANDALONE)
    mark_as_advanced(Asio_INCLUDE_DIR)
  endif()
endif()
