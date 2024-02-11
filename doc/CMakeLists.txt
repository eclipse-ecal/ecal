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

project(documentation_sphinx)

#Setup Python VENV
message(DEBUG "Configuring eCAL documentation build")
set(PYTHON_VENV_PATH "${CMAKE_CURRENT_BINARY_DIR}/_python_venv")
set(CMAKE_FOLDER "docs")

find_package(Doxygen REQUIRED)

if(NOT EXISTS "${PYTHON_VENV_PATH}")
  find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)
  message(STATUS "Creating Python ${Python3_VERSION} virtual environment at ${PYTHON_VENV_PATH}")
  execute_process(COMMAND 
    "${Python3_EXECUTABLE}" "-m"
    "venv" "${PYTHON_VENV_PATH}"
    RESULT_VARIABLE PYTHON_VENV_RESULT
  )
  message(DEBUG "Python venv RESULT_VARIABLE: ${PYTHON_VENV_RESULT}")
  if(NOT "${PYTHON_VENV_RESULT}" EQUAL 0)
    message(FATAL_ERROR 
      " Failed create Python virtual environment using 'venv'.\n"
      " You may manually create the virtual environment at the following location"
      " to bypass this step:\n"
      " ${PYTHON_VENV_PATH}"
      )
  endif()
endif()

# update the environment with VIRTUAL_ENV variable (mimic the activate script)
set(ENV{VIRTUAL_ENV} "${PYTHON_VENV_PATH}")
unset(ENV{CONDA_PREFIX})

# change the context of the search
set(Python3_FIND_VIRTUALENV "ONLY")
set(Python3_ROOT_DIR "${PYTHON_VENV_PATH}")

# unset from prior search
unset(Python3_EXECUTABLE)

# Launch a new search
find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)

message(STATUS "Documentation Python ${Python3_VERSION} executable: ${Python3_EXECUTABLE}")

set(VENV_STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/venv.stamp")

# Install documentation build dependencies
add_custom_command(OUTPUT "${VENV_STAMP_FILE}"
  COMMAND "${Python3_EXECUTABLE}" "-m" 
    "pip" "install" "--upgrade" "pip"
    "--quiet" "--require-virtualenv"
  COMMAND "${Python3_EXECUTABLE}" "-m"
    "pip" "install"
    "-r" "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
    "--quiet" "--require-virtualenv"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
  COMMENT "Installing documentation pip dependencies"
  COMMAND "${CMAKE_COMMAND}" -E touch "${VENV_STAMP_FILE}"
)
add_custom_target(documentation_install_deps DEPENDS "${VENV_STAMP_FILE}")

# This directory contains all files for creating the actual documentation
set(DOC_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/_sources)
set(PYTHON_SOURCE_DIRECTORY ${CMAKE_BINARY_DIR}/python)

#######################################
# Copy RST files to build directory
# Also display them all in the IDE
#######################################
file(GLOB_RECURSE rst_sources *.rst)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${rst_sources})

add_custom_target(documentation_copy_rst_files ALL
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/rst ${DOC_SOURCE_DIRECTORY}
  SOURCES ${rst_sources}
)

add_custom_target(documentation_copy_sample_files ALL
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/samples/ ${DOC_SOURCE_DIRECTORY}/source_code_samples
)

#######################################
# Doxygen documentation with xml output
#######################################
set(DOXYGEN_GENERATE_HTML        NO)
set(DOXYGEN_GENERATE_MAN         NO)
set(DOXYGEN_SEARCHENGINE         NO)
set(DOXYGEN_GENERATE_HTMLHELP    NO)
set(DOXYGEN_GENERATE_XML         YES)
set(DOXYGEN_EXTRACT_PRIVATE      NO)
set(DOXYGEN_ENABLE_PREPROCESSING YES)
set(DOXYGEN_MACRO_EXPANSION      YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF   YES)
set(DOXYGEN_PREDEFINED           protected=private)



set(DOXYGEN_PROJECT_NAME         "eCAL API")
set(DOXYGEN_PROJECT_BRIEF        "enhanced communication abstraction layer")
set(DOXYGEN_EXAMPLE_PATH         ../samples)
set(DOXYGEN_EXAMPLE_RECURSIVE    YES)
set(DOXYGEN_FILE_PATTERNS        "*.h")
set(DOXYGEN_RECURSIVE            NO)
set(DOXYGEN_OUTPUT_DIRECTORY     ${DOC_SOURCE_DIRECTORY})
set(DOXYGEN_XML_OUTPUT           ${DOC_SOURCE_DIRECTORY}/_doxygen)
set(DOXYGEN_DISTRIBUTE_GROUP_DOC NO)

# Allow @experimental to be used in headers to mark an API as experimental and add it to a list of
# experimental APIs.
set(DOXYGEN_ALIASES
    "experimental = \\xrefitem experimental \\\"This feature is marked as experimental\\\" \\\"Experimental List\\\""
    "experimental{1} = \\xrefitem experimental \\\"Experimental\\\" \\\"Experimental List\\\" \\1")

doxygen_add_docs(
  documentation_run_doxygen
  ./doc.hpp
  ../ecal/core/include/ecal
  ../ecal/core/include/ecal/msg
  #../ecal/core/include/ecal/cimpl
  #../lang/csharp/include
  ../contrib/ecalhdf5/include/ecalhdf5
  ../contrib/ecaltime/include
  #../ecal/metal/include/ecalmetal
  #../ecal/metal/include/ecalmetal/udp
)




#set(SPHINX_THEME sphinx_rtd_theme)

#if(NOT DEFINED SPHINX_THEME)
#    set(SPHINX_THEME default)
#endif()
 
#if(NOT DEFINED SPHINX_THEME_DIR)
#    set(SPHINX_THEME_DIR)
#endif()
 
# Sphinx cache with pickled ReST documents
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
# HTML output directory
set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
 
###################################################
# Configure the conf_py_buildvars.py.in file
#################################################### 
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf_py_buildvars.py.in"
    "${DOC_SOURCE_DIRECTORY}/conf_py_buildvars.py"
    @ONLY)
 
###################################################
# Now run the actual sphinx documentation
####################################################  
add_custom_target(documentation_sphinx ALL
    COMMAND "${Python3_EXECUTABLE}" -m sphinx
        -q -b html
        -c "${DOC_SOURCE_DIRECTORY}"
        -d "${SPHINX_CACHE_DIR}"
        -Dbreathe_projects.eCAL=${DOXYGEN_XML_OUTPUT}
        "${DOC_SOURCE_DIRECTORY}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx"
    DEPENDS 
      documentation_install_deps 
      documentation_run_doxygen 
      documentation_copy_rst_files 
      _ecal_core_py 
      _ecal_hdf5_py 
      documentation_copy_sample_files
)
    
    
# Install the generated files
install(DIRECTORY ${SPHINX_HTML_DIR}/
  DESTINATION ${eCAL_install_doc_dir}
  COMPONENT doc 
)
