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

project(hdf5)

ecal_find_package(HDF5 COMPONENTS C REQUIRED)

set(ecalhdf5_src
  src/eh5_meas.cpp
  src/eh5_meas_dir.cpp
  src/eh5_meas_file_v1.cpp
  src/eh5_meas_file_v2.cpp
  src/eh5_meas_file_v3.cpp
  src/eh5_meas_file_v4.cpp
  src/eh5_meas_file_v5.cpp
)

set(ecalhdf5_header_base
    include/ecal/measurement/imeasurement.h
    include/ecal/measurement/omeasurement.h
    include/ecalhdf5/eh5_defs.h
    include/ecalhdf5/eh5_meas.h
    include/ecalhdf5/eh5_types.h    
)

ecal_add_library(${PROJECT_NAME} ${ecalhdf5_src} ${ecalhdf5_header_base})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PRIVATE src
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${MSVC}>:_UNICODE>)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wextra>)

if (WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    hdf5::hdf5-static
  )
else()
  target_include_directories(${PROJECT_NAME} PRIVATE ${HDF5_INCLUDE_DIRS})
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${HDF5_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC eCAL::message)

ecal_install_library(${PROJECT_NAME})
install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
