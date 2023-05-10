project(measurement_mcap)

find_package(mcap REQUIRED)

ecal_add_library(measurement_mcap)
add_library(eCAL::measurement_mcap ALIAS measurement_mcap)

target_sources(measurement_mcap
  PUBLIC
    include/ecal/measurement/mcap/reader.h 
    include/ecal/measurement/mcap/writer.h 
  PRIVATE
    src/reader.cpp
    src/writer.cpp  
)

target_include_directories(measurement_mcap
  PUBLIC
    include
)

target_link_libraries(measurement_mcap 
  PRIVATE
    mcap
  PUBLIC 
    eCAL::measurement_base
)

ecal_install_library(measurement_mcap)

set_property(TARGET measurement_mcap PROPERTY FOLDER contrib/measurement)