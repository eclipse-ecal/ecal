configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/innosetup/ecal_setup.iss.in"
  "${CPACK_TOPLEVEL_DIRECTORY}/innosetup/ecal_setup.iss"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/innosetup/modpath.iss"
  "${CPACK_TOPLEVEL_DIRECTORY}/innosetup/modpath.iss"
  COPYONLY
)

file(COPY        "${CMAKE_CURRENT_LIST_DIR}/innosetup/gfx"
   DESTINATION "${CPACK_TOPLEVEL_DIRECTORY}/innosetup/"
)

find_program(ISSC_PATH
  ISCC
  PATHS
    "C:/Program Files (x86)/Inno Setup 6"
    "C:/Program Files/Inno Setup 6"
)

if (ISSC_PATH STREQUAL "ISSC_PATH-NOTFOUND")
  message(FATAL_ERROR "Unable to find Innosetup (ISCC.exe)")
else()
  message(STATUS "Found Innosetup at ${ISSC_PATH}")
  execute_process(
    COMMAND "${ISSC_PATH}" "${CPACK_TOPLEVEL_DIRECTORY}/innosetup/ecal_setup.iss"
    WORKING_DIRECTORY "${CPACK_TOPLEVEL_DIRECTORY}/innosetup"
  )
endif()