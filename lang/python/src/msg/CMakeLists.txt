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

######
# build
######
add_library(python_msg INTERFACE)

set(python_msg_files
  ecal/msg/__init__.py
  ecal/msg/common/core.py
  ecal/msg/common/serializer.py
  ecal/msg/common/__init__.py
  ecal/msg/proto/helper.py
  ecal/msg/proto/publisher.py
  ecal/msg/proto/serializer.py
  ecal/msg/proto/subscriber.py
  ecal/msg/proto/__init__.py
  ecal/msg/string/core.py
  ecal/msg/string/serializer.py
  ecal/msg/string/__init__.py
)

target_sources(python_msg
  PUBLIC    
    FILE_SET python_msg_files
    TYPE HEADERS
    BASE_DIRS .
    FILES
      ${pure_python_files}
)

#should we install the filesets or this this done via setup.toml

##########
# IDE Debugging / Runtime
###########
copy_python_code(TARGET python_msg PYTHON_FILES ${pure_python_files} PYTHON_CODE_ROOT .)

ecal_python_set_output_directory(${PROJECT_NAME})
