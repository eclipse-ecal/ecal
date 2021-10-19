cmake_minimum_required(VERSION 3.5.1)

project(tcpub)
# TODO: Remove or consider how to set these variables
#set(PROJECT_VERSION_STRING 1.0.11)
#set(PROJECT_VERSION_MAJOR 1)
#set(PROJECT_VERSION_MINOR 0)
#set(PROJECT_VERSION_PATCH 11)

set(CMAKE_CXX_STANDARD 14) #TODO: You are not supposed to do that any more, i guess
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)
find_package(asio REQUIRED)

# Public API include directory
set (includes
    include/tcpub/callback_data.h
    include/tcpub/executor.h
    include/tcpub/publisher.h
    include/tcpub/subscriber.h
    include/tcpub/subscriber_session.h
    include/tcpub/tcpub_logger.h
)

# Private source files
set(sources
    src/executor.cpp
    src/executor_impl.cpp
    src/executor_impl.h
    src/portable_endian.h
    src/publisher.cpp
    src/publisher_impl.cpp
    src/publisher_impl.h
    src/publisher_session.cpp
    src/publisher_session.h
    src/subscriber.cpp
    src/subscriber_impl.cpp
    src/subscriber_impl.h
    src/subscriber_session.cpp
    src/subscriber_session_impl.cpp
    src/subscriber_session_impl.h
    src/tcp_header.h
    src/tcpub_logger_abstraction.h
)

add_library (${PROJECT_NAME}
    ${includes}
    ${sources}
)
add_library (tcpub::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
        $<$<BOOL:${WIN32}>:ws2_32>
        $<$<BOOL:${WIN32}>:wsock32>

        # Link header-only libs (asio & recycle) as described in this workaround:
        # https://gitlab.kitware.com/cmake/cmake/-/issues/15415#note_633938
        $<BUILD_INTERFACE:asio::asio>
        $<BUILD_INTERFACE:steinwurf::recycle>
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ASIO_STANDALONE
        ASIO_DISABLE_VISIBILITY
        _WIN32_WINNT=0x0601
)

target_compile_options(${PROJECT_NAME} PRIVATE
                           $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                                -Wall -Wextra>
                           $<$<CXX_COMPILER_ID:MSVC>:
                                /W4>)


# Add own public include directory
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                  $<INSTALL_INTERFACE:include>)
target_include_directories(${PROJECT_NAME} PRIVATE src/)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION_STRING}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME ${PROJECT_NAME}
)

##################################

include(../sourcetree.cmake)
create_source_tree(${includes} ${sources})

##################################

set(TCPUB_INSTALL_CMAKE_DIR "lib/cmake/tcpub")

install(
  TARGETS ${PROJECT_NAME}
  EXPORT tcpubTargets 
  LIBRARY DESTINATION "lib" 
  ARCHIVE DESTINATION "lib"   
)

install(DIRECTORY "include/tcpub" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
install(
  EXPORT tcpubTargets 
  FILE tcpubTargets.cmake 
  DESTINATION ${TCPUB_INSTALL_CMAKE_DIR}
  NAMESPACE tcpub::
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "../cmake/tcpubConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/tcpubConfig.cmake"
  INSTALL_DESTINATION ${TCPUB_INSTALL_CMAKE_DIR}
  PATH_VARS TCPUB_INSTALL_CMAKE_DIR
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/tcpubConfig.cmake" DESTINATION ${TCPUB_INSTALL_CMAKE_DIR})

