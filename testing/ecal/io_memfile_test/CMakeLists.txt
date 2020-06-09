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

project(test_memfile)

ecal_find_package(GTest REQUIRED)

set(memfile_test_src
    src/memfile_test.cpp
    ../../../ecal/core/src/io/ecal_memfile.cpp
)

if(UNIX)
set(memfile_test_linux_src
    ../../../ecal/core/src/io/linux/ecal_memfile_linux.cpp
)
endif()

if(WIN32)
set(memfile_test_linux_src
    ../../../ecal/core/src/io/win32/ecal_memfile_win32.cpp
)
endif()

ecal_add_gtest(${PROJECT_NAME} ${memfile_test_src} ${memfile_test_linux_src} ${memfile_test_win32_src})

target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:eCAL::core,INCLUDE_DIRECTORIES>)

if(UNIX)
  target_link_libraries(${PROJECT_NAME} PRIVATE rt dl pthread)
endif()

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER testing/ecal/io)
