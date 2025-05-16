# CMakeLists.txt for EcalConfigHelper
cmake_minimum_required(VERSION 3.10)

project(EcalConfigHelper)

add_library(EcalConfigHelper STATIC
  ecal_config_helper.cpp
)

target_include_directories(EcalConfigHelper
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(EcalConfigHelper
  PUBLIC
    eCAL::core
)
