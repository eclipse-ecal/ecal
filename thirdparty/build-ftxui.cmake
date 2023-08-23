option(FTXUI_BUILD_EXAMPLES "Set to ON to build examples" OFF)

add_subdirectory(thirdparty/ftxui EXCLUDE_FROM_ALL)

# Set ftxui_VERSION_MAJOR, because it's only defined in the subdirectory scope and we cannot access it
# Reading it automatically is less error prone than setting itt by hand
file(READ thirdparty/ftxui/CMakeLists.txt content)
if(content MATCHES "VERSION ([0-9]+)\\.[0-9]+\\.[0-9]+")
  set(ftxui_VERSION_MAJOR "${CMAKE_MATCH_1}")
else()
  message(FATAL_ERROR "Couldn't read version info")
endif()