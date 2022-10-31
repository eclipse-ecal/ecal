option(FTXUI_BUILD_EXAMPLES "Set to ON to build examples" OFF)

add_subdirectory(thirdparty/ftxui)

# Upgrade Visual studio toolset version if necessary. Minimum working version is v142
# TODO: Remove this code block as soon as possible
if (MSVC AND (MSVC_TOOLSET_VERSION LESS 142))
  set_target_properties(component
    PROPERTIES
      VS_PLATFORM_TOOLSET v142
  )
  set_target_properties(screen
    PROPERTIES
      VS_PLATFORM_TOOLSET v142
  )
  set_target_properties(dom
    PROPERTIES
      VS_PLATFORM_TOOLSET v142
  )
endif()
