# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

#######################
#  Unfortunately these install functions cannot be used to install the headers
#  Each target will have to install it by themselves (e.g. install(DIRECTORY ...))
######################

function(ecal_install_library TARGET_NAME)
  if(BUILD_SHARED_LIBS)
    ecal_install_shared_library(${TARGET_NAME})
  else ()
    ecal_install_static_library(${TARGET_NAME})
  endif()
endfunction()

function(ecal_install_static_library TARGET_NAME)
  install(TARGETS ${TARGET_NAME}
  # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALCoreTargets
    ARCHIVE       DESTINATION "${eCAL_install_archive_dir}" COMPONENT sdk
    LIBRARY       DESTINATION "${eCAL_install_lib_dir}"     COMPONENT sdk
  )
  ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_bin_dir}" COMPONENT runtime)      
endfunction()

# installing shared libraries is a li
function(ecal_install_shared_library TARGET_NAME)
# Windows, RUNTIME -> .dll, ARCHIVE -> .lib, Unix: LIBRARY -> .so
  install(TARGETS ${TARGET_NAME}
  # IMPORTANT: Add the library to the "export-set"
    EXPORT eCALCoreTargets
    RUNTIME       DESTINATION "${eCAL_install_bin_dir}"         COMPONENT runtime
    LIBRARY       DESTINATION "${eCAL_install_lib_dir}"         COMPONENT sdk
    ARCHIVE       DESTINATION "${eCAL_install_archive_dyn_dir}" COMPONENT sdk
  )
  ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_bin_dir}" COMPONENT runtime)      
endfunction()

# These are libraries used by application (e.g. on Unix)
# but they are not supposed to be used by other Applications
# Hence they are not added to the export list.
function(ecal_install_private_shared_library TARGET_NAME)
  install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION "${eCAL_install_bin_dir}" COMPONENT runtime  # applies to windows .dll
    LIBRARY DESTINATION "${eCAL_install_lib_dir}" COMPONENT sdk  # applies to unix .so
  )
  ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_bin_dir}" COMPONENT runtime)      
endfunction()

# Applications are all APPS that come with the eCAL Installation
# e.g. the eCAL Monitor, eCAL Player, eCAL recorder
#
function(ecal_install_app TARGET_NAME)
  set(oneValueArgs START_MENU_NAME)
  cmake_parse_arguments(ECAL_INSTALL_APP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION  "${eCAL_install_app_dir}" COMPONENT app
  )
  set_property(INSTALL "${eCAL_install_app_dir}/$<TARGET_FILE_NAME:${TARGET_NAME}>"
    PROPERTY CPACK_START_MENU_SHORTCUTS "${ECAL_INSTALL_APP_START_MENU_NAME}"
  )

  if(UNIX AND (DEFINED ECAL_INSTALL_APP_START_MENU_NAME))
    configure_file("${CMAKE_CURRENT_LIST_DIR}/appmenu/app.desktop.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/appmenu/ecal_${TARGET_NAME}.desktop"
                   @ONLY)
    configure_file("${CMAKE_CURRENT_LIST_DIR}/appmenu/icon.png"
                   "${CMAKE_CURRENT_BINARY_DIR}/appmenu/ecal_${TARGET_NAME}.png"
                   COPYONLY)

   INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/appmenu/ecal_${TARGET_NAME}.png"
           DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps/")

    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/appmenu/ecal_${TARGET_NAME}.desktop"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/applications/")
  endif()
ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_app_dir}" COMPONENT app)       
endfunction()

function(ecal_install_gtest TARGET_NAME)
  install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION  "${eCAL_install_tests_dir}" COMPONENT testing
  )
ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_tests_dir}" COMPONENT testing)      
endfunction()


# Samples are sample applications that demonstrate eCAL capability
# They will be installed to `${prefix}/share/ecal/samples`
function(ecal_install_sample TARGET_NAME)
  install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION  "${eCAL_install_samples_dir}" COMPONENT samples
  )
ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_samples_dir}" COMPONENT samples)       
endfunction()

# Use this function to install time plugins
# We need to provide a similar function for installing custom build plugins.
function(ecal_install_time_plugin TARGET_NAME)
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION  "${eCAL_install_bin_dir}/${ECAL_TIME_PLUGIN_DIR}" COMPONENT app
    LIBRARY DESTINATION  "${eCAL_install_lib_dir}/${ECAL_TIME_PLUGIN_DIR}" COMPONENT app
  )
ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_bin_dir}/${ECAL_TIME_PLUGIN_DIR}" COMPONENT app)   
endfunction()

# Use this function to install monitor plugins
# We need to provide a similar function for installing custom build plugins.
# For some unknown reason, a MODULE dll on Windows is considered as LIBRARY, not RUNTIME
function(ecal_install_mon_plugin TARGET_NAME)
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION  "${eCAL_install_bin_dir}/${ECAL_MON_PLUGIN_DIR}" COMPONENT app
    LIBRARY DESTINATION  $<IF:$<BOOL:${WIN32}>,${eCAL_install_bin_dir}/${ECAL_MON_PLUGIN_DIR},${eCAL_install_lib_dir}/${ECAL_MON_PLUGIN_DIR}> COMPONENT app
  )
ecal_install_pdbs(TARGET ${TARGET_NAME} DESTINATION "${eCAL_install_bin_dir}/${ECAL_MON_PLUGIN_DIR}" COMPONENT app)   
endfunction()

function(ecal_install_pdbs)
if (MSVC)
  set(options        )
  set(oneValueArgs   TARGET DESTINATION COMPONENT)
  set(multiValueArgs )
  cmake_parse_arguments(INPUT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  get_target_property(TARGET_TYPE ${INPUT_TARGET} TYPE)
  if((${TARGET_TYPE} STREQUAL "SHARED_LIBRARY") OR (${TARGET_TYPE} STREQUAL "EXECUTABLE") OR (${TARGET_TYPE} STREQUAL "MODULE_LIBRARY"))
    install(FILES $<TARGET_PDB_FILE:${INPUT_TARGET}> DESTINATION ${INPUT_DESTINATION} COMPONENT ${INPUT_COMPONENT} OPTIONAL)
  elseif (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY")    
    # do nothing for static libraries, as it's not supported by CMake.
  endif()
endif()
endfunction()
