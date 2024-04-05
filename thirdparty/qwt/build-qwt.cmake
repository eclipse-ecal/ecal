include_guard(GLOBAL)

# If QWT_LIBRARY_TYPE is empty or not set, it will default to "STATIC"
if(NOT DEFINED QWT_LIBRARY_TYPE OR QWT_LIBRARY_TYPE STREQUAL "")
    set(QWT_LIBRARY_TYPE "STATIC")
endif()

ecal_disable_all_warnings()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} thirdparty/qwt EXCLUDE_FROM_ALL SYSTEM)
ecal_restore_warning_level()

set_property(TARGET qwt PROPERTY FOLDER thirdparty/qwt)