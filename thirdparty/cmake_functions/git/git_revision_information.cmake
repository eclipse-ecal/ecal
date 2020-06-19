# This function retrieves git information from the current directory
# The function will set the following variables in the parent workspace:
# IS_GIT_TREE        - The information whether or not this is a git repo
# GIT_REVISION_MAYOR - The mayor revision of the repo, as extracted from the last git tag
# GIT_REVISION_MINOR - The minor revision of the repo, as extracted from the last git tag
# GIT_REVISION_PATCH - The patch revision of the repo, as extracted from the last git tag
# GIT_DESCRIBE_TAG   - The complete tag as given by git describe --tags --dirty
function (git_revision_information)

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
  
  # No error occured, we're in a git repo!
  if (GIT_FOUND AND "${GIT_ERROR}" STREQUAL "")
    MESSAGE(STATUS "Version of this git repo: ${GIT_OUTPUT}")
    STRING(REGEX MATCH "v?([0-9]+)\\.([0-9]+)\\.([0-9]+)"  MY_OUTPUT ${GIT_OUTPUT})
    set(IS_GIT_TREE TRUE PARENT_SCOPE)
    set(GIT_REVISION_MAYOR ${CMAKE_MATCH_1}   PARENT_SCOPE)
    set(GIT_REVISION_MINOR ${CMAKE_MATCH_2}   PARENT_SCOPE)
    set(GIT_REVISION_PATCH ${CMAKE_MATCH_3}   PARENT_SCOPE)
    set(GIT_DESCRIBE_TAG   ${GIT_OUTPUT}      PARENT_SCOPE)
    set(eCAL_BUILD_DATE    ${GIT_OUTPUT_DATE} PARENT_SCOPE)
    
  # There was an error when invoking git describe - we can assume that this is not a git repo
  else (GIT_FOUND AND "${GIT_ERROR}" STREQUAL "")
    MESSAGE(STATUS "This is not a git repository, assuming source tree build")
    STRING(TIMESTAMP CURRENT_DATE %d.%m.%Y) # Use current date as build date
    set(IS_GIT_TREE FALSE PARENT_SCOPE)
    set(GIT_REVISION_MAYOR 0               PARENT_SCOPE)
    set(GIT_REVISION_MINOR 0               PARENT_SCOPE)
    set(GIT_REVISION_PATCH 0               PARENT_SCOPE)
    set(GIT_DESCRIBE_TAG   "0.0.0"         PARENT_SCOPE)
    set(eCAL_BUILD_DATE    ${CURRENT_DATE} PARENT_SCOPE)
  
  endif (GIT_FOUND AND "${GIT_ERROR}" STREQUAL "")  
  
endfunction (git_revision_information)
