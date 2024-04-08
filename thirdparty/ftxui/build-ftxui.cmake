include_guard(GLOBAL)

option(FTXUI_BUILD_EXAMPLES "Set to ON to build examples" OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ftxui thirdparty/ftxui EXCLUDE_FROM_ALL SYSTEM)

# Set ftxui_VERSION_MAJOR, because it's only defined in the subdirectory scope and we cannot access it
# Reading it automatically is less error prone than setting itt by hand
file(READ ${CMAKE_CURRENT_LIST_DIR}/ftxui/CMakeLists.txt content)
if(content MATCHES "VERSION ([0-9]+)\\.[0-9]+\\.[0-9]+")
  set(ftxui_VERSION_MAJOR "${CMAKE_MATCH_1}")
else()
  message(FATAL_ERROR "Couldn't read version info")
endif()

set_property(TARGET screen PROPERTY FOLDER thirdparty/ftxui)
set_property(TARGET dom PROPERTY FOLDER thirdparty/ftxui)
set_property(TARGET component PROPERTY FOLDER thirdparty/ftxui)