function(create_targets_protobuf)
  if (NOT TARGET protobuf::libprotobuf)

    # Backwards compatibility
    # Define camel case versions of input variables
    foreach(UPPER
      PROTOBUF_SRC_ROOT_FOLDER
      PROTOBUF_IMPORT_DIRS
      PROTOBUF_DEBUG
      PROTOBUF_LIBRARY
      PROTOBUF_PROTOC_LIBRARY
      PROTOBUF_INCLUDE_DIR
      PROTOBUF_INCLUDE_DIRS
      PROTOBUF_LIBRARY_DEBUG
      PROTOBUF_PROTOC_LIBRARY_DEBUG
      PROTOBUF_LITE_LIBRARY
      PROTOBUF_LITE_LIBRARY_DEBUG
      )
      if (DEFINED ${UPPER})
        string(REPLACE "PROTOBUF_" "Protobuf_" Camel ${UPPER})
        if (NOT DEFINED ${Camel})
          set(${Camel} ${${UPPER}})
        endif()
      endif()
    endforeach()

    add_library(protobuf::libprotobuf UNKNOWN IMPORTED)
    #message(STATUS "Include directories: ${Protobuf_INCLUDE_DIRS}")
    set_target_properties(protobuf::libprotobuf PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES "${Protobuf_INCLUDE_DIRS}")
    if (MSVC)
    set_target_properties(protobuf::libprotobuf PROPERTIES  
                INTERFACE_COMPILE_DEFINITIONS "${Protobuf_DEFINITIONS}")
    endif (MSVC)
    #message(STATUS "Debug lib: ${Protobuf_LIBRARY_DEBUG}")  
    #message(STATUS "Release lib: ${Protobuf_LIBRARY}")     
    set_target_properties(protobuf::libprotobuf PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${Protobuf_LIBRARY}"
      IMPORTED_LOCATION_DEBUG "${Protobuf_LIBRARY_DEBUG}"
      IMPORTED_LOCATION_RELEASE "${Protobuf_LIBRARY}"
      IMPORTED_IMPLIB "${Protobuf_LIBRARY}"               
      IMPORTED_IMPLIB_DEBUG "${Protobuf_LIBRARY_DEBUG}"
      IMPORTED_IMPLIB_RELEASE "${Protobuf_LIBRARY}"
    )
  endif (NOT TARGET protobuf::libprotobuf)

endfunction()
