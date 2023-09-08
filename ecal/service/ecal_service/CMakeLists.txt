cmake_minimum_required(VERSION 3.5.1)

project(ecal_service)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable default export of symbols
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

find_package(Threads REQUIRED)
find_package(asio REQUIRED)

# Public API include directory
set (includes
    include/ecal/service/client_manager.h
    include/ecal/service/client_session.h
    include/ecal/service/client_session_types.h
    include/ecal/service/error.h
    include/ecal/service/logger.h
    include/ecal/service/server.h
    include/ecal/service/server_manager.h
    include/ecal/service/server_session_types.h
    include/ecal/service/state.h
)

# Private source files
set(sources
    src/client_manager.cpp 
    src/client_session.cpp
    src/client_session_impl_base.h
    src/client_session_impl_v0.cpp
    src/client_session_impl_v0.h
    src/client_session_impl_v1.cpp
    src/client_session_impl_v1.h
    src/condition_variable_signaler.h
    src/log_defs.h
    src/log_helpers.h
    src/protocol_layout.h
    src/protocol_v0.cpp
    src/protocol_v0.h
    src/protocol_v1.cpp
    src/protocol_v1.h
    src/server.cpp
    src/server_impl.cpp
    src/server_impl.h
    src/server_manager.cpp
    src/server_session_impl_base.h
    src/server_session_impl_v0.cpp
    src/server_session_impl_v0.h
    src/server_session_impl_v1.cpp
    src/server_session_impl_v1.h
)

# Build as object library
add_library (${PROJECT_NAME} OBJECT
    ${includes}
    ${sources}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        # Link header-only libs (asio & recycle) as described in this workaround:
        # https://gitlab.kitware.com/cmake/cmake/-/issues/15415#note_633938
        $<BUILD_INTERFACE:asio::asio>
    PRIVATE
        Threads::Threads
        $<$<BOOL:${WIN32}>:ws2_32>
        $<$<BOOL:${WIN32}>:wsock32>

)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ASIO_STANDALONE
        _WIN32_WINNT=0x0601
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_compile_options(${PROJECT_NAME} PRIVATE
                           $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                                -Wall -Wextra>
                           $<$<CXX_COMPILER_ID:MSVC>:
                                /W4>)


# Add own public include directory
target_include_directories(${PROJECT_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src/
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    FOLDER ecal/service
)

##################################

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${includes}
    ${sources}
)