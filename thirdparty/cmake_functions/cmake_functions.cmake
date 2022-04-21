set (file_list_include 
  protoc_functions/protoc_generate_files.cmake
  target_definitions/targets_protobuf.cmake
)

set(file_list_no_include
  protoc_functions/protoc_generate_cpp.cmake
  protoc_functions/protoc_generate_python.cmake
)

# Set list of all files to be installed by CMake Script.
set(install_file_list
  ${file_list_include}
  ${file_list_no_include}
)

# Include all files which need to be included
foreach(f ${file_list_include})
 include(${CMAKE_CURRENT_LIST_DIR}/${f})
endforeach()
