include_guard(GLOBAL)

# Check if Npcap / Pcap++ are available as pre-downloaded .zip files
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/npcap/npcap-sdk.zip")
  set(NPCAP_SDK_ARCHIVE_URL "${CMAKE_CURRENT_LIST_DIR}/npcap/npcap-sdk.zip")
endif()
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/npcap/pcapplusplus.zip")
  set(PCAPPLUSPLUS_ARCHIVE_URL "${CMAKE_CURRENT_LIST_DIR}/npcap/pcapplusplus.zip")
endif()

# Build as static library
set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

# Let udpcap pull the dependencies
set(UDPCAP_BUILD_SAMPLES               OFF)
set(UDPCAP_THIRDPARTY_ENABLED          ON)
set(UDPCAP_THIRDPARTY_USE_BUILTIN_ASIO OFF)

# Add udpcap library from subdirectory
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/udpcap" "${eCAL_BINARY_DIR}/thirdparty/udpcap" EXCLUDE_FROM_ALL SYSTEM)

# ecaludp delay loads wpcap.dll and Ninja does not implicitly link delayimp.lib
# unlike the Visual Studio generators.
# So we explictly set it to be linked
target_link_options(udpcap PUBLIC delayimp.lib)

# Reset static / shared libs to old value
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD})
unset(BUILD_SHARED_LIBS_OLD)

# move the udpcap target to a subdirectory in the IDE
set_property(TARGET udpcap PROPERTY FOLDER thirdparty/udpcap)
