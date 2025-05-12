cmake_minimum_required(VERSION 3.10)
project(EcalConfigHelper)

find_package(eCAL REQUIRED)

# Set library version
set(ECAL_CONFIG_HELPER_VERSION 1.0.0)

# Add the source file
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(COMMON_SRC ${SRC_DIR}/ecal_config_helper.cpp)

# Create the library
add_library(EcalConfigHelper STATIC ${COMMON_SRC})

# Set include directories for build and install
target_include_directories(EcalConfigHelper 
    PUBLIC 
        $<BUILD_INTERFACE:${SRC_DIR}>
        $<INSTALL_INTERFACE:include/EcalConfigHelper>
        ${ECAL_INCLUDE_DIRS}
)

#target_link_libraries(EcalConfigHelper 
#    PUBLIC 
#        eCAL::core
#)

# Set C++ standard
set_target_properties(EcalConfigHelper PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Define installation rules
install(TARGETS EcalConfigHelper
    EXPORT EcalConfigHelperTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ecal_config_helper.h 
    DESTINATION include/EcalConfigHelper
)

# Export configuration for find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/EcalConfigHelperConfigVersion.cmake"
    VERSION ${ECAL_CONFIG_HELPER_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT EcalConfigHelperTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/EcalConfigHelperTargets.cmake"
    NAMESPACE EcalConfigHelper::
)

configure_file(EcalConfigHelperConfig.cmake.in 
    "${CMAKE_CURRENT_BINARY_DIR}/EcalConfigHelperConfig.cmake" 
    @ONLY
)

install(EXPORT EcalConfigHelperTargets
    FILE EcalConfigHelperTargets.cmake
    NAMESPACE EcalConfigHelper::
    DESTINATION lib/cmake/EcalConfigHelper
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/EcalConfigHelperConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EcalConfigHelperConfigVersion.cmake"
    DESTINATION lib/cmake/EcalConfigHelper
)
