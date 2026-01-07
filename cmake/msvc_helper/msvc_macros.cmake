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

# returns 64 or 32
macro(bit_build out)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(${out} 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(${out} 32)
endif()
endmacro()

# $(MSVC_PLATFORM)
macro(msvc_platform out)
bit_build(PLATFORM_CURRENT_BUILD)
if (PLATFORM_CURRENT_BUILD EQUAL 32)
  set(${out} "Win32")
elseif (PLATFORM_CURRENT_BUILD EQUAL 64)
  set(${out} "x64")
endif()
endmacro()

# $(MSVC_PLATFORM_TOOLSET)
macro(msvc_platform_toolset out)
if (MSVC_VERSION GREATER_EQUAL 2000)
  message(WARNING "unknown MSVC_VERSION")
elseif (MSVC_VERSION GREATER_EQUAL 1910)
  set(${out} "v141" ) 
elseif (MSVC_VERSION EQUAL 1900)
  set(${out} "v140" )  
elseif (MSVC_VERSION EQUAL 1800)
  set(${out} "v120" )
elseif (MSVC_VERSION EQUAL 1700)
  set(${out} "v110" )
elseif (MSVC_VERSION EQUAL 1600)
  set(${out} "v100" )
elseif (MSVC_VERSION} EQUAL 1500)
  set(${out} "v90" )
elseif (MSVC_VERSION EQUAL 1400)
  set(${out} "v80" )  
elseif (MSVC_VERSION EQUAL 1310)
  set(${out} "v71" )
elseif (MSVC_VERSION EQUAL 1300)
  set(${out} "v70" )
elseif (MSVC_VERSION EQUAL 1200)
  set(${out} "v60" )
endif()
endmacro()
