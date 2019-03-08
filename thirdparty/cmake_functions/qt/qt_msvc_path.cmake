# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2018 Continental Corporation
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


# Macro that determines the subfolder in a Qt installation to append the 
# CMAKE_PREFIX_PATH variable
macro(qt_msvc_path out)
if (MSVC_VERSION GREATER_EQUAL 1930)
  message(WARNING "unknown MSVC_VERSION")
elseif (MSVC_VERSION GREATER_EQUAL 1920)
  set(${out} "msvc2019" ) 
elseif (MSVC_VERSION GREATER_EQUAL 1910)
  set(${out} "msvc2017" ) 
elseif (MSVC_VERSION EQUAL 1900)
  set(${out} "msvc2015" )  
elseif (MSVC_VERSION EQUAL 1800)
  set(${out} "msvc2013" )
elseif (MSVC_VERSION EQUAL 1700)
  set(${out} "msvc2012" )
elseif (MSVC_VERSION EQUAL 1600)
  set(${out} "msvc2010" )  
else ()
  message(FATAL_ERROR "Older Visual Studio Versions are not supported")
endif()

if (CMAKE_CL_64)
  set(${out} "${${out}}_64")
endif ()
endmacro()