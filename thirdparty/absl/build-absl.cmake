include_guard(GLOBAL)

# Copied out of protobuf/cmake/abseil-cpp.cmake
if(protobuf_INSTALL)
  # When protobuf_INSTALL is enabled and Abseil will be built as a module,
  # Abseil will be installed along with protobuf for convenience.
  set(ABSL_ENABLE_INSTALL ON)
endif()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/absl" "${eCAL_BINARY_DIR}thirdparty/absl" SYSTEM)
