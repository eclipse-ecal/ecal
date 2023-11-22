function(deprecate_git_mayor_version _variable _access)
  if (_access STREQUAL "READ_ACCESS")
    message(WARNING "The variable ${_variable} is deprecated. Please use the variable GIT_REVISION_MAJOR instead. The old variable may be removed in future versions of cmake_functions.")
  endif ()
endfunction()

function(deprecate_ecal_build_date _variable _access)
  if (_access STREQUAL "READ_ACCESS")
    message(WARNING "The variable ${_variable} is deprecated. Please use the variable GIT_REVISION_DATE instead. The old variable may be removed in future versions of cmake_functions.")
  endif ()
endfunction()

# This function retrieves git information from the current directory
# The function will set the following variables in the parent workspace:
# It takes the optional value "DEFAULT", which will be parsed if the repository is not a git repository
# IS_GIT_TREE        - The information whether or not this is a git repo
# GIT_REVISION_MAJOR - The mayor revision of the repo, as extracted from the last git tag
# GIT_REVISION_MAYOR (deprecated) - The mayor revision of the repo, as extracted from the last git tag
# GIT_REVISION_MINOR - The minor revision of the repo, as extracted from the last git tag
# GIT_REVISION_PATCH - The patch revision of the repo, as extracted from the last git tag
# GIT_DESCRIBE_TAG   - The complete tag as given by git describe --tags --dirty
# GIT_REVISION_DATE  - The date when the tagged commit was made
function (git_revision_information)

  set(options )
  set(oneValueArgs DEFAULT)
  set(multiValueArgs)
  cmake_parse_arguments(GIT_REVISION "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )
                          
  if (NOT GIT_REVISION_DEFAULT)
    set(GIT_REVISION_DEFAULT "0.0.0")
  endif ()

  FIND_PACKAGE(Git)
  if (GIT_FOUND)
  # Retrieve the descriptor of this git repository and calculate the version numbers from it
  EXECUTE_PROCESS(
       COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       OUTPUT_VARIABLE GIT_OUTPUT
       ERROR_VARIABLE  GIT_ERROR
       OUTPUT_STRIP_TRAILING_WHITESPACE
   )

  # Find out the date of the respective commit as build date
  EXECUTE_PROCESS(
       COMMAND ${GIT_EXECUTABLE} show -s --format=%cd --date=format:%d.%m.%Y
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       OUTPUT_VARIABLE GIT_OUTPUT_DATE
       ERROR_VARIABLE  GIT_ERROR_DATE
       OUTPUT_STRIP_TRAILING_WHITESPACE
   )
   
  MESSAGE(STATUS "Content of the message: ${GIT_ERROR}")
  endif (GIT_FOUND)
  
  if (GIT_FOUND AND "${GIT_ERROR}" STREQUAL "")
    MESSAGE(STATUS "Version of this git repo: ${GIT_OUTPUT}")
    set(IS_GIT_TREE TRUE PARENT_SCOPE)
  else ()
    MESSAGE(STATUS "This is not a git repository, assuming source tree build")
    set(IS_GIT_TREE FALSE PARENT_SCOPE)
    set(GIT_OUTPUT ${GIT_REVISION_DEFAULT})

    STRING(TIMESTAMP GIT_OUTPUT_DATE %d.%m.%Y)
  endif ()
  
  STRING(REGEX MATCH "v?([0-9]+)\\.([0-9]+)\\.([0-9]+)"  MY_OUTPUT ${GIT_OUTPUT})
  set(IS_GIT_TREE TRUE PARENT_SCOPE)
  set(GIT_REVISION_MAJOR ${CMAKE_MATCH_1}   PARENT_SCOPE)    
  set(GIT_REVISION_MAYOR ${CMAKE_MATCH_1}   PARENT_SCOPE)
  set(GIT_REVISION_MINOR ${CMAKE_MATCH_2}   PARENT_SCOPE)
  set(GIT_REVISION_PATCH ${CMAKE_MATCH_3}   PARENT_SCOPE)
  set(GIT_DESCRIBE_TAG   ${GIT_OUTPUT}      PARENT_SCOPE)
  set(GIT_REVISION_DATE  ${GIT_OUTPUT_DATE} PARENT_SCOPE)
  set(eCAL_BUILD_DATE    ${GIT_OUTPUT_DATE} PARENT_SCOPE)
     
  # deprecate the old variable!
  variable_watch(GIT_REVISION_MAYOR deprecate_git_mayor_version)
  variable_watch(eCAL_BUILD_DATE deprecate_ecal_build_date)
  
endfunction (git_revision_information)
