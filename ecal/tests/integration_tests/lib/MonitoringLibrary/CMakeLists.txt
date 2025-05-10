cmake_minimum_required(VERSION 3.10)
project(MonitoringLibrary)

find_package(eCAL REQUIRED)

set(MONITORING_LIBRARY_VERSION 1.0.0)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES MonitoringLibrary.cpp)

add_library(MonitoringLibrary STATIC ${SRC_FILES})

target_include_directories(MonitoringLibrary 
    PUBLIC 
        $<BUILD_INTERFACE:${SRC_DIR}>
        $<INSTALL_INTERFACE:include/MonitoringLibrary>
        ${ECAL_INCLUDE_DIRS}
)

target_link_libraries(MonitoringLibrary 
    eCAL::core
)

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

install(EXPORT MonitoringLibraryTargets
    FILE MonitoringLibraryTargets.cmake
    NAMESPACE MonitoringLibrary::
    DESTINATION lib/cmake/MonitoringLibrary
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/MonitoringLibraryConfigVersion.cmake"
    DESTINATION lib/cmake/MonitoringLibrary
)
