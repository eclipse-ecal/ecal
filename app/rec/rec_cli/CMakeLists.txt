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

ecal_find_package(tclap REQUIRED)
ecal_find_package(Protobuf REQUIRED)
ecal_find_package(tclap REQUIRED)

set(PROJECT_NAME rec)

set(source_files
    src/ecal_rec_cli.cpp
    src/ecal_rec_service.cpp
    src/ecal_rec_service.h
)

if(WIN32)
    set(win_src
        resources/resource.h
        resources/win32_resources.rc
    )
endif(WIN32)

ecal_add_app_console(${PROJECT_NAME} ${source_files} ${win_src})

create_targets_protobuf()

target_link_libraries(${PROJECT_NAME} 
  PUBLIC
  tclap::tclap
  eCAL::rec_core
  eCAL::core
  tclap::tclap
  eCAL::utils
)

ecal_install_app(${PROJECT_NAME})

if(MSVC)
    msvc_source_tree(
        ${source_files}
        ${win_src}
    )
endif(MSVC)

target_link_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/ignore:4098,4099>)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER app/rec)
