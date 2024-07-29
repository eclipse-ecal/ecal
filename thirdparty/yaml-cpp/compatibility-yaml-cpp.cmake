# Create targets to be compatible with yaml-cpp < 0.8.0
macro(yaml_cpp_create_compatibility_targets)
  if (NOT TARGET yaml-cpp::yaml-cpp AND TARGET yaml-cpp)
    # ALIASing a imported non-global library requires CMake 3.18 so we do this
    add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED)
    target_link_libraries(yaml-cpp::yaml-cpp INTERFACE yaml-cpp)
  endif()
endmacro()