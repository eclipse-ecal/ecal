include_guard(GLOBAL)

add_library(_ecal_warnings INTERFACE)

set(compiler_flags "")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  message(STATUS "MSVC detected - Adding flags")
  set(compiler_flags "/MP" "/W4")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
  message(STATUS "Setting GNU/Clang flags")
  set(compiler_flags "-Wall" "-Wextra")
else()
  message(WARNING "Unknown compiler, will not set warning flags")
endif()
target_compile_options(_ecal_warnings INTERFACE "${compiler_flags}")

unset(compiler_flags)

function(ecal_add_compiler_warnings TARGET_NAME)
  target_link_libraries("${TARGET_NAME}" PRIVATE $<BUILD_INTERFACE:_ecal_warnings>)
endfunction()
