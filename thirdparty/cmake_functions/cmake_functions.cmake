set (file_list_include 
  git/git_revision_information.cmake
  msvc_helper/msvc_macros.cmake
  msvc_helper/msvc_sourcetree.cmake
  protoc_functions/protoc_generate_files.cmake
  target_definitions/targets_hdf5.cmake
  target_definitions/targets_protobuf.cmake
  qt/qt_msvc_path.cmake
  qt/qt_windeployqt.cmake
)

set(file_list_no_include
  protoc_functions/protoc_generate_cpp.cmake
  qt/qt_windeployqt_threadsafe_cmake.bat.in
)

# Set list of all files to be installed by CMake Script.
set(file_list
  ${file_list_include}
  ${file_list_no_include}
)

# Include all files which need to be included
foreach(f ${file_list_include})
 include(${CMAKE_CURRENT_LIST_DIR}/${f})
endforeach()
