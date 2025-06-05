cmake_minimum_required(VERSION 3.10)
project(message_validation)

find_package(Protobuf REQUIRED)
find_package(eCAL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ECALCONFIGHELPER_PATH)
  set(ECALCONFIGHELPER_PATH ${CMAKE_CURRENT_LIST_DIR}/../../lib/EcalConfigHelper)
else()
  add_subdirectory(${ECALCONFIGHELPER_PATH} ${CMAKE_BINARY_DIR}/EcalConfigHelper)  
endif()

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/test_message.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

set(COMMON_SRC ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(message_publisher message_publisher.cpp ${COMMON_SRC})
target_include_directories(message_publisher
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(message_publisher
  PRIVATE
    eCAL::core
    protobuf::libprotobuf
    EcalConfigHelper 
)

add_executable(message_subscriber message_subscriber.cpp ${COMMON_SRC})
target_include_directories(message_subscriber
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Protobuf_INCLUDE_DIRS}
)
target_link_libraries(message_subscriber
  PRIVATE
    eCAL::core
    protobuf::libprotobuf
    EcalConfigHelper
)
