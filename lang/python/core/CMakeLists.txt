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

project(_ecal_core_py)

######
# build
######
python_add_library(${PROJECT_NAME} MODULE WITH_SOABI
  src/ecal_clang.cpp
  src/ecal_clang.h
  src/ecal_wrap.cxx
)

set(python_files
  ecal/__init__.py
  ecal/core/__init__.py
  ecal/core/core.py
  ecal/core/publisher.py
  ecal/core/service.py
  ecal/core/subscriber.py
  ecal/proto/__init__.py
  ecal/proto/helper.py
)

target_sources(${PROJECT_NAME}
  PUBLIC    
    FILE_SET ecal_core_python_files
    TYPE HEADERS
    BASE_DIRS .
    FILES
      ${python_files}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::core
)

# we're using only definitions from eCAL::core_c but no functionality.
# This is a bit hacky, however, we will hopefully switch to nanobind soon.
target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<TARGET_PROPERTY:eCAL::core_c,INCLUDE_DIRECTORIES>
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)


########
# installation
########
install(TARGETS ${PROJECT_NAME} core
  RUNTIME DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
  LIBRARY DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL NAMELINK_SKIP
)


##############
# IDE appearance
##############
set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER lang/python/core
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Python Files" FILES ${python_files})


##########
# IDE Debugging / Runtime
###########
copy_python_code(TARGET ${PROJECT_NAME} PYTHON_FILES ${python_files} PYTHON_CODE_ROOT .)

ecal_python_set_output_directory(${PROJECT_NAME})