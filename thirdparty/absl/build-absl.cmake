include_guard(GLOBAL)

# Abseil is installed alongside protobuf, as protobuf's exported targets depend on it.
# Abseil does not name an install component; we put it in its own one to keep it out of the runtime component.
function(ecal_add_absl)
  set(ABSL_ENABLE_INSTALL ON)
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME absl)
  add_subdirectory("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/absl" "${eCAL_BINARY_DIR}/thirdparty/absl" SYSTEM)
endfunction()

ecal_add_absl()
