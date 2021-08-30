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

if (WIN32)
    if (TARGET hdf5::hdf5-shared)
        add_custom_target(copy_hdf5_dll ALL
            COMMAND cmake -E copy_if_different "$<TARGET_FILE:hdf5::hdf5-shared>" "${PYTHON_BINARY_MODULE_DIR}"
            COMMENT "Copy hdf5 DLL to python directory"
            DEPENDS hdf5::hdf5-shared
        )
        set_property(TARGET copy_hdf5_dll PROPERTY FOLDER lang/python/hdf5)
    endif()
endif()

# ==========================================

project(_ecal_hdf5_py)

find_package(Python COMPONENTS Development)

set(ecal_lang_py_src
    src/ecalhdf5_wrap.cxx
)

ecal_add_python_module(${PROJECT_NAME} SOURCES ${ecal_lang_py_src} PYTHON_CODE ${CMAKE_CURRENT_SOURCE_DIR}/ecal)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Python::Python
        eCAL::hdf5
)

if (WIN32)
    if (TARGET hdf5::hdf5-shared)
        add_dependencies(${PROJECT_NAME} copy_hdf5_dll)
    endif()
endif()

if(MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "/ignore:4098,4099")
endif(MSVC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/python/hdf5)

if(BUILD_STANDALONE_PY_WHEEL AND NOT WIN32)
  find_package(patchelf REQUIRED)
  add_custom_command(TARGET ${PROJECT_NAME} 
                     POST_BUILD
                     COMMAND "${CMAKE_COMMAND}" ARGS "-DRUNTIME_FILE=\"$<TARGET_FILE:${PROJECT_NAME}>\"" "-DRUNTIME_DEP_FILTER=\"hdf5|sz|aec\"" "-DRUNTIME_DEPLOY_DIR=\"${PYTHON_BINARY_MODULE_DIR}\"" -P "${CMAKE_SOURCE_DIR}/cmake/deploy_runtime_deps.cmake"
                     COMMAND "${PATCHELF_COMMAND}" ARGS "--remove-rpath" "\"$<TARGET_FILE:${PROJECT_NAME}>\""
                     COMMAND "${PATCHELF_COMMAND}" ARGS "--force-rpath" "--set-rpath" "\\$$ORIGIN" "\"$<TARGET_FILE:${PROJECT_NAME}>\"")
endif()

if(ECAL_INCLUDE_PY_SAMPLES)
  if(WIN32)

    include_external_msproject(ecal_hdf5_py ${CMAKE_CURRENT_SOURCE_DIR}/ecal_hdf5_py.pyproj)
    set_property(TARGET ecal_hdf5_py PROPERTY FOLDER lang/python/hdf5)

  endif()
endif()
