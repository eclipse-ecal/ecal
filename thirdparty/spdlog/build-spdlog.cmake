include_guard(GLOBAL)

set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "My option" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "My option" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/spdlog thirdparty/spdlog EXCLUDE_FROM_ALL SYSTEM)
set_property(TARGET spdlog PROPERTY FOLDER thirdparty/spdlog)