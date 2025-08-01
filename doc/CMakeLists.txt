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

project(documentation_sphinx)

#Setup Python VENV
# find_package(PythonInterp REQUIRED)

# set(PYTHON_VENV_PATH "${CMAKE_BINARY_DIR}/_python_venv")

# message(STATUS "Creating Python venv at ${PYTHON_VENV_PATH}")
# exec_program("${PYTHON_EXECUTABLE}"
    # ARGS -m venv ${PYTHON_VENV_PATH}
# )

# update the environment with VIRTUAL_ENV variable (mimic the activate script)
# set (ENV{VIRTUAL_ENV} "${PYTHON_VENV_PATH}")
# change the context of the search
# set (Python_FIND_VIRTUALENV FIRST)
# unset Python3_EXECUTABLE because it is also an input variable (see documentation, Artifacts Specification section)
# unset (Python3_EXECUTABLE)
# Launch a new search
# find_package(PythonInterp REQUIRED)

# message(STATUS "========Python executable: ${PYTHON_EXECUTABLE}")
# message(STATUS "========PYTHON_VENV_PATH : ${PYTHON_VENV_PATH}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(Sphinx REQUIRED)
find_package(Doxygen REQUIRED)

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
set_property(TARGET documentation_copy_rst_files  PROPERTY FOLDER docs)

add_custom_target(documentation_copy_sample_files ALL
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/samples                                              ${DOC_SOURCE_DIRECTORY}/source_code_samples

  # pubsub blob samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/ecal/samples/cpp/pubsub/binary                       ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/binary/blob
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/c/samples/pubsub/binary                         ${DOC_SOURCE_DIRECTORY}/source_code_samples/c/binary/blob
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/csharp/Eclipse.eCAL.Core.Samples/pubsub         ${DOC_SOURCE_DIRECTORY}/source_code_samples/csharp/binary/blob
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/core/pubsub                      ${DOC_SOURCE_DIRECTORY}/source_code_samples/python_legacy/binary/blob
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/nanobind_core/pubsub             ${DOC_SOURCE_DIRECTORY}/source_code_samples/python/binary/blob

  # pubsub string samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/serialization/string/samples/pubsub                  ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/string/hello
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/c/samples/pubsub/string                         ${DOC_SOURCE_DIRECTORY}/source_code_samples/c/string/hello
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/csharp/Eclipse.eCAL.String.Samples/pubsub       ${DOC_SOURCE_DIRECTORY}/source_code_samples/csharp/string/hello
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/core/pubsub                      ${DOC_SOURCE_DIRECTORY}/source_code_samples/python_legacy/string/hello
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/nanobind_core/pubsub             ${DOC_SOURCE_DIRECTORY}/source_code_samples/python/string/hello

  # pubsub protobuf samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/serialization/protobuf/samples/pubsub                ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/protobuf/person
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/csharp/Eclipse.eCAL.Protobuf.Samples/pubsub     ${DOC_SOURCE_DIRECTORY}/source_code_samples/csharp/protobuf/person
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/core/pubsub                      ${DOC_SOURCE_DIRECTORY}/source_code_samples/python_legacy/protobuf/person
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/nanobind_core/pubsub             ${DOC_SOURCE_DIRECTORY}/source_code_samples/python/protobuf/person

  # pubsub capnproto samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/serialization/capnproto/samples/pubsub               ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/capnproto/addressbook

  # pubsub flatbuffers samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/serialization/flatbuffers/samples/pubsub             ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/flatbuffers/monster


  # config samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/ecal/samples/cpp/misc                                ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/misc
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/c/samples/misc                                  ${DOC_SOURCE_DIRECTORY}/source_code_samples/c/misc

  # sercli binary samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/ecal/samples/cpp/services                            ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/binary/mirror
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/c/samples/services                              ${DOC_SOURCE_DIRECTORY}/source_code_samples/c/binary/mirror
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/csharp/Eclipse.eCAL.Core.Samples/service        ${DOC_SOURCE_DIRECTORY}/source_code_samples/csharp/binary/mirror
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/core/service                     ${DOC_SOURCE_DIRECTORY}/source_code_samples/python_legacy/binary/mirror
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/lang/python/samples/nanobind_core/service            ${DOC_SOURCE_DIRECTORY}/source_code_samples/python/binary/mirror

  # sercli protobuf samples
  COMMAND cmake -E copy_directory ${ECAL_PROJECT_ROOT}/serialization/protobuf/samples/services              ${DOC_SOURCE_DIRECTORY}/source_code_samples/cpp/protobuf/math
  )

set_property(TARGET documentation_copy_sample_files  PROPERTY FOLDER docs)

add_custom_target(documentation_copy_config_file
  COMMENT "Copying generated configuration file to documentation directory"
  DEPENDS run_ecal_generate_config
)
set_property(TARGET documentation_copy_config_file  PROPERTY FOLDER docs)

if (DEFINED ECAL_GENERATED_YAML_PATH)
  add_custom_command(TARGET documentation_copy_config_file
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_SOURCE_DIRECTORY}/configuration_files
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ECAL_GENERATED_YAML_PATH} ${DOC_SOURCE_DIRECTORY}/configuration_files
    COMMAND ${CMAKE_COMMAND} -E echo "Configuration file copied successfully."
    COMMAND ${CMAKE_COMMAND} -E echo "Configuration file not found, skipping copy." IF NOT EXISTS ${ECAL_GENERATED_YAML_PATH}
  )
endif()

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
  #../ecal/core/include/ecal/msg
  #../ecal/core/include/ecal/cimpl
  #../lang/csharp/include
  ../contrib/ecalhdf5/include/ecalhdf5
  ../contrib/ecaltime/include
)

set_property(TARGET documentation_run_doxygen PROPERTY FOLDER docs)



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
    #${SPHINX_APIDOC} -f
    #-o ${DOC_SOURCE_DIRECTORY}/_apidoc/ ${PYTHON_SOURCE_DIRECTORY}
    COMMAND ${SPHINX_EXECUTABLE}
        -q -b html -j auto
        -c "${DOC_SOURCE_DIRECTORY}"
        -d "${SPHINX_CACHE_DIR}"
        -Dbreathe_projects.eCAL=${DOXYGEN_XML_OUTPUT}
        "${DOC_SOURCE_DIRECTORY}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx"
    DEPENDS documentation_run_doxygen documentation_copy_rst_files documentation_copy_sample_files documentation_copy_config_file
)
    
set_property(TARGET documentation_sphinx  PROPERTY FOLDER docs)
    
# Install the generated files
install(DIRECTORY ${SPHINX_HTML_DIR}/
  DESTINATION ${eCAL_install_doc_dir}
  COMPONENT doc 
)

