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

cmake_minimum_required(VERSION 3.17) #CMAKE_CURRENT_FUNCTION_LIST_DIR has been added in 3.17

# Define the function to add the PowerShell script as a post-build command
function(add_exclusive_command_as_postbuild target)
    # Get the directory where the CMakeLists.txt is located (same as PowerShell script)
    get_filename_component(SCRIPT_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}" ABSOLUTE)

    # Define the PowerShell script path
    set(PS1_SCRIPT "${SCRIPT_DIR}/exclusive_command.ps1")

    # Add the post-build command for the target
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${PS1_SCRIPT}" ${ARGN}
        COMMENT "Running PowerShell script after building ${target}..."
    )
endfunction()