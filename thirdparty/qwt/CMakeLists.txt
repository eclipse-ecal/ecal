cmake_minimum_required(VERSION 3.13)
project(qwt VERSION 6.2.0)

find_package(Qt5 REQUIRED COMPONENTS Widgets OpenGL Svg Concurrent PrintSupport)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE qwt_headers
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/qwt/src/*.h
)

file(GLOB_RECURSE qwt_sources
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/qwt/src/*.cpp
)

add_library(qwt SHARED ${qwt_headers} ${qwt_sources})
add_library(qwt::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(qwt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/qwt/src)
target_link_libraries(qwt PUBLIC Qt5::Widgets Qt5::OpenGL Qt5::Svg Qt5::Concurrent Qt5::PrintSupport)
target_compile_definitions(qwt PRIVATE QT_DLL QWT_DLL QWT_MAKEDLL QWT_MOC_INCLUDE) 


if(WIN32)
    # Deploy Qt DLLs in the binary folder. This is necessary for starting the application from whithin the IDE without having to copy QtCore.dll, QtWidgets.dll etc. by hand each time
    qt_add_windeployqt_postbuild(--no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw --pdb "$<TARGET_FILE:qwt>")
endif()

# Export targets and install them
install(TARGETS qwt
  EXPORT qwtTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT qwt_runtime
)

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

if(WIN32)
install(CODE
    "
    set(_file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/qwt.dll)
    execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E
                env PATH=\"${_qt_bin_dir}\" \"${WINDEPLOYQT_EXECUTABLE}\"
                    --dry-run
                    --no-compiler-runtime
                    --no-angle
                    --no-opengl-sw
                    --list mapping
                    \${_file}
            OUTPUT_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    separate_arguments(_files WINDOWS_COMMAND \${_output})
    while(_files)
            list(GET _files 0 _src)
            list(GET _files 1 _dest)
            execute_process(
                COMMAND \"${CMAKE_COMMAND}\" -E
                    copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/bin/\${_dest}\"
            )
            list(REMOVE_AT _files 0 1)
    endwhile()
    "
)
endif()