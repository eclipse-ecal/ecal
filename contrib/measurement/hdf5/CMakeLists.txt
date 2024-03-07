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

project(measurement_hdf5)

find_package(HDF5 COMPONENTS C REQUIRED)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} 
  include/ecal/measurement/hdf5/reader.h
  include/ecal/measurement/hdf5/writer.h
  src/escape.cpp
  src/escape.h
  src/hdf5_helper.cpp
  src/hdf5_helper.h
  src/reader.cpp
  src/reader_caching.cpp
  src/reader_caching.h
  src/reader_hdf5_v5.cpp
  src/reader_hdf5_v5.h
  src/reader_hdf5_v6.cpp
  src/reader_hdf5_v6.h
  src/reader_impl.h
  src/reader_merging.cpp
  src/reader_merging.h
  src/reader_unescaping.cpp
  src/reader_unescaping.h
  src/writer.cpp
  src/writer_escaping.cpp
  src/writer_escaping.h
  src/writer_hdf5_v5.cpp
  src/writer_hdf5_v5.h
  src/writer_hdf5_v6.cpp
  src/writer_hdf5_v6.h
  src/writer_impl.h
  src/writer_splitting.cpp
  src/writer_splitting.h
)

add_library(eCAL::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} 
  PUBLIC
    eCAL::measurement_base
)

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

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${MSVC}>:_UNICODE>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

ecal_install_library(${PROJECT_NAME})

install(DIRECTORY
   "include/" DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT sdk
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER contrib)
