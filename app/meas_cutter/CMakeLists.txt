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

project(meas_cutter)

find_package(Threads REQUIRED)
find_package(tclap REQUIRED)
find_package(yaml-cpp REQUIRED)

include(${ECAL_PROJECT_ROOT}/thirdparty/yaml-cpp/compatibility-yaml-cpp.cmake)
yaml_cpp_create_compatibility_targets()

set(meas_cutter_src
    src/main.cpp
    src/utils.h
    src/ecal_meas_cutter_globals.h
    src/ecal_meas_cutter.h
    src/ecal_meas_cutter.cpp
    src/config_file_parser.h
    src/config_file_parser.cpp
    src/thread_pool.h
    src/thread_pool.cpp
    src/measurement_worker.h
    src/measurement_worker.cpp
    src/measurement_converter.h
    src/measurement_converter.cpp
    src/measurement_importer.h
    src/measurement_importer.cpp
    src/measurement_exporter.h
    src/measurement_exporter.cpp
    src/logger.h
    src/logger.cpp
)

ecal_add_app_console(${PROJECT_NAME} ${meas_cutter_src})

target_include_directories(${PROJECT_NAME}
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${PROJECT_NAME}   yaml-cpp::yaml-cpp
                                        tclap::tclap
                                        eCAL::ecal-utils
                                        eCAL::measurement_hdf5
                                        Threads::Threads)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14) 

ecal_install_app(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/ecal_meas)
