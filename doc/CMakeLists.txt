find_package(Doxygen REQUIRED)

# Find prerequisites for generating the documentation
if(WIN32)
  # The HTML Help Workshop is a Microsoft-provided tools used to generate the CHM files.
  find_program(HHC_EXECUTABLE
    NAMES hhc hhc.exe
    PATH_SUFFIXES "HTML Help Workshop"
    HINTS ENV HHC_LOCATION
    DOC "HTML Help Workshop executable")

  if(HHC_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "hhc.exe not found")
  endif()
  set(DOXYGEN_HHC_LOCATION ${HHC_EXECUTABLE})
endif()

if(WIN32)
  set(DOXYGEN_GENERATE_HTML        YES)
  set(DOXYGEN_GENERATE_MAN         NO)
  set(DOXYGEN_SEARCHENGINE         NO)
  set(DOXYGEN_GENERATE_HTMLHELP    YES)
else()
  set(DOXYGEN_GENERATE_HTML        NO)
  set(DOXYGEN_GENERATE_MAN         YES)
endif()

set(DOXYGEN_EXTRACT_PRIVATE      NO)
set(DOXYGEN_ENABLE_PREPROCESSING YES)
set(DOXYGEN_MACRO_EXPANSION      YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF   YES)
set(DOXYGEN_PREDEFINED           protected=private)

# Documentation of eCAL CPP API
if(WIN32)
  set(DOXYGEN_CHM_FILE ../eCAL.chm)
endif()

set(DOXYGEN_PROJECT_NAME         "eCAL C++ API")
set(DOXYGEN_PROJECT_BRIEF        "enhanced communication abstraction layer")
set(DOXYGEN_EXAMPLE_PATH         ../samples)
set(DOXYGEN_EXAMPLE_RECURSIVE    YES)
set(DOXYGEN_FILE_PATTERNS        "*.h")
set(DOXYGEN_RECURSIVE            NO)
set(DOXYGEN_HTML_OUTPUT          "html_cpp")
doxygen_add_docs(
  documentation_cpp
  ./doc.hpp
  ../ecal/core/include/ecal
  ../ecal/core/include/ecal/msg
)

# Documentation of eCAL C API
if(WIN32)
  set(DOXYGEN_PROJECT_NAME         "eCAL C API")
  set(DOXYGEN_PROJECT_BRIEF        "enhanced communication abstraction layer")
  set(DOXYGEN_CHM_FILE             ../eCALC.chm)
  set(DOXYGEN_EXAMPLE_PATH         "")
  set(DOXYGEN_EXAMPLE_RECURSIVE    NO)
  set(DOXYGEN_FILE_PATTERNS        "*.h")
  set(DOXYGEN_RECURSIVE            NO)
  set(DOXYGEN_HTML_OUTPUT          "html_c")
endif()
doxygen_add_docs(
  documentation_c
  ../ecal/core/include/ecal/cimpl
)

set_property(TARGET documentation_cpp     PROPERTY FOLDER docs)
set_property(TARGET documentation_c       PROPERTY FOLDER docs)

if (WIN32)
#install(FILES
#
#  COMPONENT doc
#)
endif (WIN32)
