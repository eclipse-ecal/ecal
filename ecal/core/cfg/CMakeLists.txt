# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
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

project(ecal_generate_config)

# Set the name of the YAML file to be generated
set(ECAL_YAML "ecal.yaml")
set(TIME_YAML "ecaltime.yaml")

set(ECAL_YAML_PATH ${CMAKE_CURRENT_BINARY_DIR}/${ECAL_YAML})
set(TIME_YAML_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TIME_YAML})

add_executable(${PROJECT_NAME} 
  ${ECAL_CORE_PROJECT_ROOT}/core/cfg/generate_configuration_yaml.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/configuration_writer.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/default_configuration.cpp
  ${ECAL_CORE_PROJECT_ROOT}/core/src/config/ecal_path_processing.cpp
)

# Set the output paths for the generated YAML files
set(YAML_PATHS
  ${ECAL_YAML_PATH}
  ${TIME_YAML_PATH}
)

# Add a custom command to generate the YAML file
add_custom_command(
  OUTPUT  ${YAML_PATHS}                            # Specify the output file
  COMMAND $<TARGET_FILE:${PROJECT_NAME}> --dump    # Command to run the executable
  DEPENDS ${PROJECT_NAME}                          # Ensure the executable is built before running the command
  COMMENT "Generating ${YAML_PATHS}"               # Comment to display during the build process
  VERBATIM                                         # Ensure command arguments are passed as-is
)

# Add a custom target that depends on the generated YAML file
add_custom_target(run_${PROJECT_NAME} ALL
  DEPENDS ${YAML_PATHS}       # Ensure the custom command is executed
)

# Ensure the executable is built before running the custom target
add_dependencies(run_${PROJECT_NAME} ${PROJECT_NAME})

# Specify include directories for the executable
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${ECAL_CORE_PROJECT_ROOT}/core/src
  ${ECAL_CORE_PROJECT_ROOT}/core/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  eCAL::core
  eCAL::ecal-utils
)

# Select the correct ecal config directory on Linux and Windows
if(UNIX)
  set(ECAL_SHARED_CONFIG ${eCAL_install_config_dir})
elseif(WIN32)
  set(ECAL_SHARED_CONFIG ${eCAL_install_config_dir})
else()
  message(STATUS "Unsupported OS, not installing configs")
endif()

# Install the generated YAML file if not cross-compiling
if(ECAL_SHARED_CONFIG AND NOT CMAKE_CROSSCOMPILING)
  install(
    FILES
      ${ECAL_YAML_PATH}             # Files to install
    DESTINATION
      ${ECAL_SHARED_CONFIG}         # Destination directory
    COMPONENT configuration         # Installation component
  )

  if (ECAL_BUILD_TIMEPLUGINS)
    install(
      FILES
        ${TIME_YAML_PATH}             # Files to install
      DESTINATION
        ${ECAL_SHARED_CONFIG}         # Destination directory
      COMPONENT configuration         # Installation component
    )
  endif()
endif()

ecal_install_app(${PROJECT_NAME})