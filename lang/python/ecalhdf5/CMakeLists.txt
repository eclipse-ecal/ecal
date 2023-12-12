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

#if (WIN32)
#    if (TARGET hdf5::hdf5-shared)
#        add_custom_target(copy_hdf5_dll ALL
#            COMMAND cmake -E copy_if_different "$<TARGET_FILE:hdf5::hdf5-shared>" "${PYTHON_BINARY_MODULE_DIR}"
#            COMMENT "Copy hdf5 DLL to python directory"
#            DEPENDS hdf5::hdf5-shared
#        )
#        set_property(TARGET copy_hdf5_dll PROPERTY FOLDER lang/python/hdf5)
#    endif()
#endif()

# ==========================================

project(_ecal_hdf5_py)

find_package(Python3 REQUIRED COMPONENTS Development.Module)

set(ecal_lang_py_src
    src/ecalhdf5_wrap.cxx
)

Python3_add_library(${PROJECT_NAME} MODULE ${ecal_lang_py_src})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        eCAL::hdf5
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

install(TARGETS ${PROJECT_NAME}
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_MODULE_NAME})

install(PROGRAMS $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_MODULE_NAME})

if(MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "/ignore:4098,4099")
endif(MSVC)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER lang/python/hdf5)
