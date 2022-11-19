# Build as static library
set(CMAKE_BUILD_SHARED_LIBS_OLD ${CMAKE_BUILD_SHARED_LIBS})
set(CMAKE_BUILD_SHARED_LIBS OFF)


# Let udpcap pull the dependencies
set(UDPCAP_BUILD_SAMPLES               OFF)
set(UDPCAP_THIRDPARTY_ENABLED          ON)
set(UDPCAP_THIRDPARTY_USE_BUILTIN_ASIO OFF)

# Add udpcap library from subdirectory
add_subdirectory(thirdparty/udpcap/ EXCLUDE_FROM_ALL)
add_library(udpcap::udpcap ALIAS udpcap)

# Reset static / shared libs to old value
set(CMAKE_BUILD_SHARED_LIBS ${CMAKE_BUILD_SHARED_LIBS_OLD})
unset(CMAKE_BUILD_SHARED_LIBS_OLD)

# move the udpcap target to a subdirectory in the IDE
set_property(TARGET udpcap PROPERTY FOLDER lib/udpcap)
