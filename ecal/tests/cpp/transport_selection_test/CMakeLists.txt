project(test_transport_selection)

find_package(GTest REQUIRED)

ecal_add_gtest(${PROJECT_NAME} src/transport_selection_test.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../core/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../../core/include")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

ecal_install_gtest(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER tests/cpp/pubsub)