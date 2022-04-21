cmake_minimum_required(VERSION 3.0)

include(protoc_functions.cmake)

project(ProtocFunctions VERSION 0.0.1)

if (MSVC)
# Variable definitions
set(protoc_functions_install_cmake_dir   cmake)
else (MSVC)
set(protoc_functions_install_cmake_dir   lib/cmake/${PROJECT_NAME}-${PROJECT_VERSION})
endif (MSVC)
set(protoc_functions_config              ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake)
set(protoc_functions_config_version      ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "cmake/${PROJECT_NAME}Config.cmake.in"
    "${protoc_functions_config}"
    INSTALL_DESTINATION "${protoc_functions_install_cmake_dir}" 
    PATH_VARS protoc_functions_install_cmake_dir
)  

write_basic_package_version_file(
  ${protoc_functions_config_version}
  VERSION ${ProtocFunctions_VERSION}
  COMPATIBILITY SameMajorVersion
)


install(FILES 
  "${protoc_functions_config}"
  "${protoc_functions_config_version}"
  "protoc_functions.cmake"
  DESTINATION "${protoc_functions_install_cmake_dir}"
) 

foreach (f ${install_file_list})
  get_filename_component( dir ${f} DIRECTORY )
  install( FILES ${f} DESTINATION "${protoc_functions_install_cmake_dir}/${dir}" )
endforeach()

include(cmake/cpack_variables.cmake)
