cmake_minimum_required(VERSION 3.16)

project(dynamic_threadpool)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable default export of symbols
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

find_package(Threads REQUIRED)

# Public API include directory
set (includes
    include/dynamic_threadpool/dynamic_threadpool.h
)

# Private source files
set(sources
    src/dynamic_threadpool.cpp
    src/dynamic_threadpool_impl.cpp
    src/dynamic_threadpool_impl.h
)

# Build as object library
add_library (${PROJECT_NAME} OBJECT
    ${includes}
    ${sources}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
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
    FOLDER core/dynamic_threadpool
)

##################################

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES 
    ${includes}
    ${sources}
)