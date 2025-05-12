cmake_minimum_required(VERSION 3.10)
project(MonitoringLibrary)

find_package(eCAL REQUIRED)

# Set library version
set(MONITORING_LIBRARY_VERSION 1.0.0)

# Add the source file
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES MonitoringLibrary.cpp)

# Create the library
add_library(MonitoringLibrary STATIC ${SRC_FILES})

# Set include directories for build and install
target_include_directories(MonitoringLibrary 
    PUBLIC 
        $<BUILD_INTERFACE:${SRC_DIR}>
        $<INSTALL_INTERFACE:include/MonitoringLibrary>
        ${eCAL_INCLUDE_DIRS}
)

# Link against eCAL to ensure the include directories are set correctly
#target_link_libraries(MonitoringLibrary 
#   PUBLIC 
#        eCAL::core
#)

# Set C++ standard
set_target_properties(MonitoringLibrary PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Define installation rules
install(TARGETS MonitoringLibrary
    EXPORT MonitoringLibraryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES MonitoringLibrary.h 
    DESTINATION include/MonitoringLibrary
)

# Export configuration for find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryConfigVersion.cmake"
    VERSION ${MONITORING_LIBRARY_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT MonitoringLibraryTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryTargets.cmake"
    NAMESPACE MonitoringLibrary::
)

configure_file(MonitoringLibraryConfig.cmake.in 
    "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryConfig.cmake" 
    @ONLY
)

install(EXPORT MonitoringLibraryTargets
    FILE MonitoringLibraryTargets.cmake
    NAMESPACE MonitoringLibrary::
    DESTINATION lib/cmake/MonitoringLibrary
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryConfigVersion.cmake"
    DESTINATION lib/cmake/MonitoringLibrary
)
