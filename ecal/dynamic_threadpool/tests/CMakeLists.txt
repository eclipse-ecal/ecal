project(test_dynamic_threadpool)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

set(sources
  src/dynamic_threadpool_test.cpp
  src/atomic_signalable.h
)

ecal_add_gtest(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    dynamic_threadpool
    GTest::gtest_main)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

ecal_install_gtest(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER core/dynamic_threadpool/tests)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${sources}
)
