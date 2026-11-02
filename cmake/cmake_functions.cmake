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

# Files that are included by this file (they define the helper functions / macros)
set (ecal_cmake_functions_include
  git/git_revision_information.cmake
  msvc_helper/msvc_macros.cmake
  protoc_functions/protoc_generate_files.cmake
  target_definitions/targets_protobuf.cmake
)

if(WIN32)
  list(APPEND ecal_cmake_functions_include
    qt/qt_windeployqt.cmake
    qt/exclusive_command.cmake
    qt/qt_msvc_path.cmake
  )
endif()

# Files that are only used by the included files, but must be available next to them
set(ecal_cmake_functions_no_include
  protoc_functions/protoc_generate_cpp.cmake
  protoc_functions/protoc_generate_python.cmake
)

if(WIN32)
  list(APPEND ecal_cmake_functions_no_include
    qt/qt_windeployqt_threadsafe_cmake.bat.in
    qt/exclusive_command.ps1
  )
endif()

# All files (relative to this file) that must be installed with the eCAL SDK
set(ecal_cmake_functions_files
  ${ecal_cmake_functions_include}
  ${ecal_cmake_functions_no_include}
)

foreach(_ecal_cmake_functions_file ${ecal_cmake_functions_include})
 include(${CMAKE_CURRENT_LIST_DIR}/${_ecal_cmake_functions_file})
endforeach()
