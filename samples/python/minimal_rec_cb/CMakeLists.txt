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

project(minimal_rec_cb)

ecal_find_package(eCAL REQUIRED)

set(PROJECT_GROUP minimal)

if(ECAL_INCLUDE_PY_SAMPLES)
  if(WIN32)

    include_external_msproject(${PROJECT_NAME}_py ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pyproj)
    set_property(TARGET ${PROJECT_NAME}_py PROPERTY FOLDER samples/python/${PROJECT_GROUP})

  endif()
endif()
