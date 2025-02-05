cmake_minimum_required(VERSION 3.5)

include(cmake_functions.cmake)

project(CMakeFunctions VERSION 0.4.1)

include(GNUInstallDirs)
if (MSVC)
# Variable definitions
set(cmake_functions_install_cmake_dir   cmake)
else (MSVC)
set(cmake_functions_install_cmake_dir   "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}-${PROJECT_VERSION}")
endif (MSVC)
set(cmake_functions_config              ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake)
set(cmake_functions_config_version      ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "cmake/${PROJECT_NAME}Config.cmake.in"
    "${cmake_functions_config}"
    INSTALL_DESTINATION "${cmake_functions_install_cmake_dir}" 
    PATH_VARS cmake_functions_install_cmake_dir
)  

write_basic_package_version_file(
  ${cmake_functions_config_version}
  VERSION ${CMakeFunctions_VERSION}
  COMPATIBILITY SameMajorVersion
)


install(FILES 
  "${cmake_functions_config}"
  "${cmake_functions_config_version}"
  "cmake_functions.cmake"
  DESTINATION "${cmake_functions_install_cmake_dir}"
) 

foreach (f ${file_list})
  get_filename_component( dir ${f} DIRECTORY )
  install( FILES ${f} DESTINATION "${cmake_functions_install_cmake_dir}/${dir}" )
endforeach()

include(cmake/cpack_variables.cmake)
