include_guard(GLOBAL)

set(ECALUDP_ENABLE_NPCAP        ${ECAL_CORE_NPCAP_SUPPORT})
set(ECALUDP_BUILD_SAMPLES       OFF)
set(ECALUDP_BUILD_TESTS         OFF)
set(ECALUDP_USE_BUILTIN_ASIO    OFF)
set(ECALUDP_USE_BUILTIN_RECYCLE OFF)
set(ECALUDP_USE_BUILTIN_UDPCAP  OFF)
set(ECALUDP_LIBRARY_TYPE        STATIC)

# Add ecaludp library from subdirectory
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ecaludp" "${eCAL_BINARY_DIR}/thirdparty/ecaludp" EXCLUDE_FROM_ALL SYSTEM)

# move the ecaludp target to a subdirectory in the IDE
set_property(TARGET ecaludp PROPERTY FOLDER thirdparty/ecaludp)
