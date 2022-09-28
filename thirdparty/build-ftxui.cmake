option(FTXUI_BUILD_EXAMPLES "Set to ON to build examples" OFF)

add_subdirectory(thirdparty/ftxui)

# Upgrade Visual studio toolset version if necessary. Minimum working version is v142
if(MSVC AND NOT CMAKE_VS_PLATFORM_TOOLSET STREQUAL "")
  string(SUBSTRING "${CMAKE_VS_PLATFORM_TOOLSET}" 1 -1 VS_TOOLSET_NUMBER)
  if ("${VS_TOOLSET_NUMBER}" LESS "142")
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
endif()
