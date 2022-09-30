set (PROJECT_NAME debscripts)

if(UNIX)
configure_file("${CMAKE_CURRENT_LIST_DIR}/postinst.in"
               "${CMAKE_CURRENT_BINARY_DIR}/postinst"
               @ONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/postrm.in"
               "${CMAKE_CURRENT_BINARY_DIR}/postrm"
               @ONLY)
endif()