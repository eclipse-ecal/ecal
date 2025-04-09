include_guard(GLOBAL)

set(ABSL_ENABLE_INSTALL "ON")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/absl" thirdparty/absl SYSTEM)
