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

set(ECAL_YAML "ecal.yaml")
set(ECALTIME_INI "ecaltime.ini")

# Merge all ecaltime config files
# -> The ecaltime.ini will only contain valid settings for the time modules that were actually compiled.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${ECALTIME_INI} "")
foreach(ECALTIME_CONFIG ${ECALTIME_CONFIG_FILES})
  file(READ ${ECALTIME_CONFIG} CONTENTS)
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${ECALTIME_INI} "${CONTENTS}")
endforeach()



# Select the correct ecal config directory on Linux and Windows
if(UNIX)
  set(ECAL_SHARED_CONFIG ${eCAL_install_config_dir})
elseif(WIN32)
  set(ECAL_SHARED_CONFIG ${eCAL_install_config_dir})
else()
  message(STATUS "Unsupported OS, not installing configs")
endif()

configure_file(
  ${ECAL_YAML}
  ${ECAL_YAML}
)

# Install the configs
if(ECAL_SHARED_CONFIG AND NOT CMAKE_CROSSCOMPILING)
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${ECAL_YAML}
    DESTINATION
      ${ECAL_SHARED_CONFIG}
    COMPONENT configuration
  )
  if(BUILD_TIME)
    install(
      FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${ECALTIME_INI}
      DESTINATION
        ${ECAL_SHARED_CONFIG}
      COMPONENT configuration        
    )
  endif()
endif()
