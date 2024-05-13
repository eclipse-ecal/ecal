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

project(_ecal_hdf5_py)

python_add_library(${PROJECT_NAME} MODULE WITH_SOABI
  src/ecalhdf5_wrap.cxx
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    eCAL::hdf5
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

set_target_properties(${PROJECT_NAME} PROPERTIES
  FOLDER lang/python/hdf5
)

if(TARGET hdf5-shared)
  install(TARGETS ${PROJECT_NAME} hdf5-shared
    RUNTIME DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
    LIBRARY DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL NAMELINK_SKIP
  )
endif()

install(TARGETS ${PROJECT_NAME}
  DESTINATION ecal COMPONENT python EXCLUDE_FROM_ALL
)

if(ECAL_INCLUDE_PY_SAMPLES)
  if(WIN32)

    include_external_msproject(ecal_hdf5_py ${CMAKE_CURRENT_SOURCE_DIR}/ecal_hdf5_py.pyproj)
    set_property(TARGET ecal_hdf5_py PROPERTY FOLDER lang/python/hdf5)

  endif()
endif()
