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

project(_ecal_hdf5_py)

######
# build
######
python_add_library(${PROJECT_NAME} MODULE WITH_SOABI
  src/ecalhdf5_wrap.cxx
)

set(python_files
  ecal/measurement/hdf5.py
  ecal/measurement/measurement.py
  ecal/measurement/writer.py
  ecal/measurement/__init__.py
)

target_sources(${PROJECT_NAME}
  PUBLIC    
    FILE_SET ecal_hdf5_python_files
    TYPE HEADERS
    BASE_DIRS .
    FILES
      ${python_files}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::hdf5
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

########
# installation
########
if(TARGET hdf5-shared)
  install(TARGETS ${PROJECT_NAME} hdf5-shared
    RUNTIME DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
    LIBRARY DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL NAMELINK_SKIP
  )
endif()

install(TARGETS ${PROJECT_NAME}
  DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
)


##############
# IDE appearance
##############
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Python Files" FILES ${python_files})
set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER lang/python/hdf5
)

##########
# IDE Debugging / Runtime
###########
copy_python_code(TARGET ${PROJECT_NAME} PYTHON_FILES ${python_files} PYTHON_CODE_ROOT .)

ecal_python_set_output_directory(${PROJECT_NAME})