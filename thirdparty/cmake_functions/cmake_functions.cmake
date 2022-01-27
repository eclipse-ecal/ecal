set (file_list_include 
  git/git_revision_information.cmake
  msvc_helper/msvc_macros.cmake
  protoc_functions/protoc_generate_files.cmake
  target_definitions/targets_protobuf.cmake
)

if(WIN32)
  list(APPEND file_list_include
    qt/qt_msvc_path.cmake
    qt/qt_windeployqt.cmake
  )
endif()

set(file_list_no_include
  protoc_functions/protoc_generate_cpp.cmake
  protoc_functions/protoc_generate_python.cmake
)

if(WIN32)
  list(APPEND file_list_no_include
    qt/qt_windeployqt_threadsafe_cmake.bat.in
  )
endif()

# Set list of all files to be installed by CMake Script.
set(file_list
  ${file_list_include}
  ${file_list_no_include}
)

# Include all files which need to be included
foreach(f ${file_list_include})
 include(${CMAKE_CURRENT_LIST_DIR}/${f})
endforeach()
