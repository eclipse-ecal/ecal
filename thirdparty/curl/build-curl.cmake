include_guard(GLOBAL)

if(WIN32)
  set(CURL_USE_SCHANNEL ON CACHE BOOL "Use native SSL on Windows" FORCE)
endif()

set(BUILD_CURL_EXE OFF CACHE BOOL "Don't build the curl executable" FORCE)
set(ENABLE_CURL_MANUAL  OFF CACHE BOOL "Disable built-in manual" FORCE)
set(PICKY_COMPILER  OFF CACHE BOOL "Disable extra compiler options" FORCE)
set(BUILD_LIBCURL_DOCS  OFF CACHE BOOL "Disable libcurl man pages" FORCE)
set(BUILD_MISC_DOCS  OFF CACHE BOOL "Disable misc man pages" FORCE)
set(BUILD_EXAMPLES  OFF CACHE BOOL "Disable curl examples" FORCE)

# All targets made by curl will go in the correct IDE folder
set(CMAKE_FOLDER "thirdparty/curl")

ecal_disable_all_warnings()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/curl thirdparty/curl EXCLUDE_FROM_ALL SYSTEM)
ecal_restore_warning_level()

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
