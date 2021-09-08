message("Runtime file                : ${RUNTIME_FILE}")
message("Runtime dependency filter   : ${RUNTIME_DEP_FILTER}")
message("Runtime deployment directory: ${RUNTIME_DEPLOY_DIR}")

file(GET_RUNTIME_DEPENDENCIES
     LIBRARIES "${RUNTIME_FILE}"
     RESOLVED_DEPENDENCIES_VAR _r_deps
     UNRESOLVED_DEPENDENCIES_VAR _u_deps
    )  
  
foreach(_file ${_r_deps})
  if(_file MATCHES "${RUNTIME_DEP_FILTER}")
    message("Deploying dependency ${_file}")
    file(INSTALL
      DESTINATION "${RUNTIME_DEPLOY_DIR}"
      TYPE SHARED_LIBRARY
      FOLLOW_SYMLINK_CHAIN
      FILES "${_file}"
    )
  endif()
endforeach()

foreach(_file ${_u_deps})
  message(WARNING "Unresolved dependency detected: ${_file}")
endforeach()
