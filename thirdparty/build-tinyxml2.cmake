set(BUILD_SHARED_LIBS OFF CACHE BOOL "My option" FORCE)
set(DBUILD_STATIC_LIBS ON CACHE BOOL "My option" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
add_subdirectory(thirdparty/tinyxml2 EXCLUDE_FROM_ALL)

if (NOT TARGET tinyxml2::tinyxml2)
  add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)
  get_target_property(is_imported tinyxml2 IMPORTED)
  if (NOT is_imported)
    # Disable warnings for third-party lib
    target_compile_options(tinyxml2 PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif ()
endif ()