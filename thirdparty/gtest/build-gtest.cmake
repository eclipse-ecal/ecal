include_guard(GLOBAL)

# Googletest automatically forces MT instead of MD if we do not set this option.
if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "My option" FORCE)
  set(BUILD_GMOCK ON CACHE BOOL "My option" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "My option" FORCE)
endif()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/googletest thirdparty/googletest EXCLUDE_FROM_ALL SYSTEM)
if(NOT TARGET GTest::gtest)
  add_library(GTest::gtest ALIAS gtest)
endif()
if(NOT TARGET GTest::gtest_main)
  add_library(GTest::gtest_main ALIAS gtest_main)
endif()

set_property(TARGET gtest PROPERTY FOLDER thirdparty/gtest)
set_property(TARGET gtest_main PROPERTY FOLDER thirdparty/gtest)
