cmake_minimum_required(VERSION 3.10)
project(message_validation)

find_package(Protobuf REQUIRED)
find_package(eCAL REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(COMMON_SRC ${SRC_DIR}/ecal_config_helper.cpp)

# Protobuf setup
set(PROTO_FILE ${SRC_DIR}/test_message.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

# Add generated Protobuf files to common source
list(APPEND COMMON_SRC ${PROTO_SRCS} ${PROTO_HDRS})

# Collect all publisher and subscriber source files
file(GLOB TEST_SRC "${SRC_DIR}/*_publisher.cpp" "${SRC_DIR}/*_subscriber.cpp")

foreach(SRC_FILE ${TEST_SRC})
    get_filename_component(BASENAME ${SRC_FILE} NAME_WE)

    if(${BASENAME} MATCHES "_publisher$")
        set(TARGET_NAME ${BASENAME})
        add_executable(${TARGET_NAME} ${SRC_FILE} ${COMMON_SRC})
        target_include_directories(${TARGET_NAME} PRIVATE ${SRC_DIR} ${BIN_DIR} ${Protobuf_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} PRIVATE eCAL::core protobuf::libprotobuf)
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER tests/integration/message_validation)
    elseif(${BASENAME} MATCHES "_subscriber$")
        set(TARGET_NAME ${BASENAME})
        add_executable(${TARGET_NAME} ${SRC_FILE} ${COMMON_SRC})
        target_include_directories(${TARGET_NAME} PRIVATE ${SRC_DIR} ${BIN_DIR} ${Protobuf_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} PRIVATE eCAL::core protobuf::libprotobuf)
        set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER tests/integration/message_validation)
    endif()
endforeach()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
