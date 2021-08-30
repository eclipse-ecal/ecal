set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "My option" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "My option" FORCE)
add_subdirectory(thirdparty/spdlog)

if (NOT TARGET spdlog::spdlog)
  add_library(spdlog::spdlog ALIAS spdlog)
  get_target_property(is_imported spdlog IMPORTED)
  if (NOT is_imported)
    # Disable warnings for third-party lib
    target_compile_options(spdlog PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W0 /wd4996>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
    )
  endif()
endif()