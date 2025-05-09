include_guard(GLOBAL)

function(ecal_get_default_compiler_warnings cxx_out_var)
  set(cxx_compiler_flags "")

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    message(STATUS "MSVC detected - Adding flags")
    set(cxx_compiler_flags "/MP" "/W4")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
    message(STATUS "Setting GNU/Clang flags")
    set(cxx_compiler_flags "-Wall" "-Wextra")
  else()
    message(WARNING "Unknown compiler, will not set warning flags")
  endif()

  set("${cxx_out_var}" "${cxx_compiler_flags}" PARENT_SCOPE)
endfunction()
