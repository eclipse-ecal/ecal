if(WIN32)
  set(CMAKE_USE_WINSSL ON CACHE BOOL "Use native SSL on Windows" FORCE)
endif()

set(BUILD_CURL_EXE OFF CACHE BOOL "Don't build the curl executable" FORCE)
set(ENABLE_MANUAL  OFF CACHE BOOL "Disable built-in manual" FORCE)

add_subdirectory(thirdparty/curl)

if (NOT TARGET CURL::libcurl)
  add_library(CURL::libcurl ALIAS libcurl)
  get_target_property(is_imported libcurl IMPORTED)
  if (NOT is_imported)
    # Disable Warnings
    target_compile_options(libcurl PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif()
endif()