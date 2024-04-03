include_guard(GLOBAL)

#tinyxml cannot be configured with debug postfixes.
set(CMAKE_DEBUG_POSTFIX            )
set(CMAKE_MINSIZEREL_POSTFIX       )
set(CMAKE_RELWITHDEBINFO_POSTFIX   )

set(BUILD_SHARED_LIBS OFF CACHE BOOL "My option" FORCE)
set(DBUILD_STATIC_LIBS ON CACHE BOOL "My option" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "My option" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "My option" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tinyxml2 thirdparty/tinyxml2 EXCLUDE_FROM_ALL SYSTEM)
set_property(TARGET tinyxml2 PROPERTY FOLDER thirdparty/tinyxml2)