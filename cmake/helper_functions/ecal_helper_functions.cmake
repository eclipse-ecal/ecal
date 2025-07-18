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

# This function will set the output names of the target according to eCAL conventions.
function(ecal_get_platform_toolset)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(eCAL_VS_TOOLSET "x64" PARENT_SCOPE)
  else()
    set(eCAL_VS_TOOLSET "Win32" PARENT_SCOPE)
  endif()
endfunction()


# This function will mark the executable as a Windows GUI application on a Windows System
function(ecal_set_subsystem_windows TARGET_NAME)
  if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
      LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS"
      LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS"
      LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
      LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
  endif()
endfunction()

# This function will mark the executable as a Windows console application on a Windows System
function(ecal_set_subsystem_console TARGET_NAME)
  if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
      LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE"
      LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE"
      LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
  endif()
endfunction()

# This function resolves a alias target to the real target 
function(ecal_resolve_alias_target TARGET_NAME OUT_VAR)
  get_target_property(_real "${TARGET_NAME}" ALIASED_TARGET)
  if(_real)
    set(${OUT_VAR} "${_real}" PARENT_SCOPE)
  else()
    set(${OUT_VAR} "${TARGET_NAME}" PARENT_SCOPE)
  endif()
endfunction()


macro(ecal_disable_all_warnings)
  if(MSVC)
    message(STATUS "supress thirdparty warnings for windows platform ..")
    set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
    endif()
  endif()
endmacro()

macro(ecal_restore_warning_level)
  if(MSVC)
    message(STATUS "reset thirdparty warnings for windows platform ..")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_OLD}")
  endif()
endmacro()

macro(ecal_variable_push var_name)
  list(APPEND ecal_${var_name}_old "${${var_name}}")
endmacro()

macro(ecal_variable_pop var_name)
  list(POP_BACK ecal_${var_name}_old "${var_name}")
endmacro()