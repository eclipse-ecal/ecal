find_program(PATCHELF_COMMAND
	     NAMES patchelf)

if(PATCHELF_COMMAND STREQUAL PATCHELF_COMMAND-NOTFOUND)
  message(FATAL_ERROR "Could not find patchelf executable.")
  set(PATCHELF_FOUND FALSE)
else()
  message(STATUS "Found patchelf: ${PATCHELF_COMMAND}")
  set(PATCHELF_FOUND TRUE)
endif()
