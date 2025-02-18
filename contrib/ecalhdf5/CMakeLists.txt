# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2024 Continental Corporation
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

project(hdf5 LANGUAGES C CXX)

if(NOT CMAKE_CROSSCOMPILING)
  find_package(HDF5 COMPONENTS C REQUIRED)
  find_package(Threads REQUIRED)
else()
  find_library(hdf5_path NAMES hdf5 REQUIRED PATH_SUFFIXES hdf5/serial)
  find_path(hdf5_include NAMES hdf5.h PATH_SUFFIXES hdf5/serial REQUIRED)  
  set(HDF5_LIBRARIES "${hdf5_path};m;dl;z;sz;pthread")
  set(HDF5_INCLUDE_DIRS "${hdf5_include}")
endif()

set(ecalhdf5_src
    src/datatype_helper.cpp
    src/datatype_helper.h
    src/eh5_meas_api_v2.cpp
    src/eh5_meas_api_v3.cpp
    src/eh5_meas_dir.cpp
    src/eh5_meas_dir.h
    src/eh5_meas_file_v1.cpp
    src/eh5_meas_file_v1.h
    src/eh5_meas_file_v2.cpp
    src/eh5_meas_file_v2.h
    src/eh5_meas_file_v3.cpp
    src/eh5_meas_file_v3.h
    src/eh5_meas_file_v4.cpp
    src/eh5_meas_file_v4.h
    src/eh5_meas_file_v5.cpp
    src/eh5_meas_file_v5.h
    src/eh5_meas_file_v6.cpp
    src/eh5_meas_file_v6.h
    src/eh5_meas_file_writer_v5.cpp
    src/eh5_meas_file_writer_v5.h
    src/eh5_meas_file_writer_v6.cpp
    src/eh5_meas_file_writer_v6.h
    src/eh5_meas_impl.h
    src/hdf5_helper.h
    src/hdf5_helper.cpp
    src/escape.cpp
    src/escape.h
)

set(ecalhdf5_header_base
    include/ecalhdf5/eh5_defs.h
    include/ecalhdf5/eh5_meas.h
    include/ecalhdf5/eh5_meas_api_v2.h
    include/ecalhdf5/eh5_meas_api_v3.h
    include/ecalhdf5/eh5_types.h
)

ecal_add_library(${PROJECT_NAME} ${ecalhdf5_src} ${ecalhdf5_header_base})
add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PRIVATE .
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${MSVC}>:_UNICODE>
  PUBLIC
    ECAL_EH5_API_VERSION=2
  )

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wextra>)

target_link_libraries(${PROJECT_NAME} 
  PUBLIC
    eCAL::measurement_base
  PRIVATE  
    eCAL::ecal-utils
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_link_libraries(${PROJECT_NAME} PUBLIC eCAL::measurement_base)
if (${ECAL_LINK_HDF5_SHARED})
  if (TARGET hdf5::hdf5-shared)
    target_link_libraries(${PROJECT_NAME} PRIVATE
      hdf5::hdf5-shared
    )
  # conan target names are different
  elseif (TARGET HDF5::C)
    target_link_libraries(${PROJECT_NAME} PRIVATE
      HDF5::C
    )
  else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${HDF5_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
      ${HDF5_LIBRARIES}
    )
  endif()
else()
  if (TARGET hdf5::hdf5-static)
    target_link_libraries(${PROJECT_NAME} PRIVATE
      hdf5::hdf5-static
    )
  # conan target names are different
  elseif (TARGET HDF5::C)
    target_link_libraries(${PROJECT_NAME} PRIVATE
      HDF5::C
    )
  else()
    message(FATAL_ERROR "eCAL build configured to link HDF5 statically, however static libs are not available. Consider building with ECAL_LINK_HDF5_SHARED=ON") 
  endif()
endif()


source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${ecalhdf5_src}
        ${ecalhdf5_header_base}
)

ecal_install_library(${PROJECT_NAME})
install(DIRECTORY
    "include/" DESTINATION "${eCAL_install_include_dir}" COMPONENT sdk
    FILES_MATCHING PATTERN "*.h")

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
