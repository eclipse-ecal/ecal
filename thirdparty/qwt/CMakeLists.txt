cmake_minimum_required(VERSION 3.14)

# Allow the install command to use generator expressions
if(POLICY CMP0087)
    cmake_policy(SET CMP0087 NEW)
endif()

project(qwt VERSION 6.2.0)

# Legacy Qt5 (pre 5.15) support as suggested by teh Qt Documentation:
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGL Svg Concurrent PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGL Svg Concurrent PrintSupport)

# For Qt6, we need the OpenGLWidgets
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets )
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(qwt_headers
    qwt/src/qwt.h
    qwt/src/qwt_abstract_legend.h
    qwt/src/qwt_abstract_scale.h
    qwt/src/qwt_abstract_scale_draw.h
    qwt/src/qwt_abstract_slider.h
    qwt/src/qwt_analog_clock.h
    qwt/src/qwt_arrow_button.h
    qwt/src/qwt_axis.h
    qwt/src/qwt_axis_id.h
    qwt/src/qwt_bezier.h
    qwt/src/qwt_clipper.h
    qwt/src/qwt_color_map.h
    qwt/src/qwt_column_symbol.h
    qwt/src/qwt_compass.h
    qwt/src/qwt_compass_rose.h
    qwt/src/qwt_counter.h
    qwt/src/qwt_curve_fitter.h
    qwt/src/qwt_date.h
    qwt/src/qwt_date_scale_draw.h
    qwt/src/qwt_date_scale_engine.h
    qwt/src/qwt_dial.h
    qwt/src/qwt_dial_needle.h
    qwt/src/qwt_dyngrid_layout.h
    qwt/src/qwt_event_pattern.h
    qwt/src/qwt_global.h
    qwt/src/qwt_graphic.h
    qwt/src/qwt_interval.h
    qwt/src/qwt_interval_symbol.h
    qwt/src/qwt_knob.h
    qwt/src/qwt_legend.h
    qwt/src/qwt_legend_data.h
    qwt/src/qwt_legend_label.h
    qwt/src/qwt_magnifier.h
    qwt/src/qwt_math.h
    qwt/src/qwt_matrix_raster_data.h
    qwt/src/qwt_null_paintdevice.h
    qwt/src/qwt_painter.h
    qwt/src/qwt_painter_command.h
    qwt/src/qwt_panner.h
    qwt/src/qwt_picker.h
    qwt/src/qwt_picker_machine.h
    qwt/src/qwt_pixel_matrix.h
    qwt/src/qwt_plot.h
    qwt/src/qwt_plot_abstract_barchart.h
    qwt/src/qwt_plot_abstract_canvas.h
    qwt/src/qwt_plot_barchart.h
    qwt/src/qwt_plot_canvas.h
    qwt/src/qwt_plot_curve.h
    qwt/src/qwt_plot_dict.h
    qwt/src/qwt_plot_directpainter.h
    # qwt/src/qwt_plot_glcanvas.h # Not compatible with Qt6
    qwt/src/qwt_plot_graphicitem.h
    qwt/src/qwt_plot_grid.h
    qwt/src/qwt_plot_histogram.h
    qwt/src/qwt_plot_intervalcurve.h
    qwt/src/qwt_plot_item.h
    qwt/src/qwt_plot_layout.h
    qwt/src/qwt_plot_legenditem.h
    qwt/src/qwt_plot_magnifier.h
    qwt/src/qwt_plot_marker.h
    qwt/src/qwt_plot_multi_barchart.h
    qwt/src/qwt_plot_opengl_canvas.h
    qwt/src/qwt_plot_panner.h
    qwt/src/qwt_plot_picker.h
    qwt/src/qwt_plot_rasteritem.h
    qwt/src/qwt_plot_renderer.h
    qwt/src/qwt_plot_rescaler.h
    qwt/src/qwt_plot_scaleitem.h
    qwt/src/qwt_plot_seriesitem.h
    qwt/src/qwt_plot_shapeitem.h
    qwt/src/qwt_plot_spectrocurve.h
    qwt/src/qwt_plot_spectrogram.h
    qwt/src/qwt_plot_svgitem.h
    qwt/src/qwt_plot_textlabel.h
    qwt/src/qwt_plot_tradingcurve.h
    qwt/src/qwt_plot_vectorfield.h
    qwt/src/qwt_plot_zoneitem.h
    qwt/src/qwt_plot_zoomer.h
    qwt/src/qwt_point_3d.h
    qwt/src/qwt_point_data.h
    qwt/src/qwt_point_mapper.h
    qwt/src/qwt_point_polar.h
    qwt/src/qwt_polar.h
    qwt/src/qwt_polar_canvas.h
    qwt/src/qwt_polar_curve.h
    qwt/src/qwt_polar_fitter.h
    qwt/src/qwt_polar_grid.h
    qwt/src/qwt_polar_item.h
    qwt/src/qwt_polar_itemdict.h
    qwt/src/qwt_polar_layout.h
    qwt/src/qwt_polar_magnifier.h
    qwt/src/qwt_polar_marker.h
    qwt/src/qwt_polar_panner.h
    qwt/src/qwt_polar_picker.h
    qwt/src/qwt_polar_plot.h
    qwt/src/qwt_polar_renderer.h
    qwt/src/qwt_polar_spectrogram.h
    qwt/src/qwt_raster_data.h
    qwt/src/qwt_round_scale_draw.h
    qwt/src/qwt_samples.h
    qwt/src/qwt_sampling_thread.h
    qwt/src/qwt_scale_div.h
    qwt/src/qwt_scale_draw.h
    qwt/src/qwt_scale_engine.h
    qwt/src/qwt_scale_map.h
    qwt/src/qwt_scale_widget.h
    qwt/src/qwt_series_data.h
    qwt/src/qwt_series_store.h
    qwt/src/qwt_slider.h
    qwt/src/qwt_spline.h
    qwt/src/qwt_spline_basis.h
    qwt/src/qwt_spline_cubic.h
    qwt/src/qwt_spline_curve_fitter.h
    qwt/src/qwt_spline_local.h
    qwt/src/qwt_spline_parametrization.h
    qwt/src/qwt_spline_pleasing.h
    qwt/src/qwt_spline_polynomial.h
    qwt/src/qwt_symbol.h
    qwt/src/qwt_system_clock.h
    qwt/src/qwt_text.h
    qwt/src/qwt_text_engine.h
    qwt/src/qwt_text_label.h
    qwt/src/qwt_thermo.h
    qwt/src/qwt_transform.h
    qwt/src/qwt_vectorfield_symbol.h
    qwt/src/qwt_weeding_curve_fitter.h
    qwt/src/qwt_wheel.h
    qwt/src/qwt_widget_overlay.h
)

set(qwt_sources
    qwt/src/qwt.cpp
    qwt/src/qwt_abstract_legend.cpp
    qwt/src/qwt_abstract_scale.cpp
    qwt/src/qwt_abstract_scale_draw.cpp
    qwt/src/qwt_abstract_slider.cpp
    qwt/src/qwt_analog_clock.cpp
    qwt/src/qwt_arrow_button.cpp
    qwt/src/qwt_bezier.cpp
    qwt/src/qwt_clipper.cpp
    qwt/src/qwt_color_map.cpp
    qwt/src/qwt_column_symbol.cpp
    qwt/src/qwt_compass.cpp
    qwt/src/qwt_compass_rose.cpp
    qwt/src/qwt_counter.cpp
    qwt/src/qwt_curve_fitter.cpp
    qwt/src/qwt_date.cpp
    qwt/src/qwt_date_scale_draw.cpp
    qwt/src/qwt_date_scale_engine.cpp
    qwt/src/qwt_dial.cpp
    qwt/src/qwt_dial_needle.cpp
    qwt/src/qwt_dyngrid_layout.cpp
    qwt/src/qwt_event_pattern.cpp
    qwt/src/qwt_graphic.cpp
    qwt/src/qwt_interval.cpp
    qwt/src/qwt_interval_symbol.cpp
    qwt/src/qwt_knob.cpp
    qwt/src/qwt_legend.cpp
    qwt/src/qwt_legend_data.cpp
    qwt/src/qwt_legend_label.cpp
    qwt/src/qwt_magnifier.cpp
    qwt/src/qwt_math.cpp
    qwt/src/qwt_matrix_raster_data.cpp
    qwt/src/qwt_null_paintdevice.cpp
    qwt/src/qwt_painter.cpp
    qwt/src/qwt_painter_command.cpp
    qwt/src/qwt_panner.cpp
    qwt/src/qwt_picker.cpp
    qwt/src/qwt_picker_machine.cpp
    qwt/src/qwt_pixel_matrix.cpp
    qwt/src/qwt_plot.cpp
    qwt/src/qwt_plot_abstract_barchart.cpp
    qwt/src/qwt_plot_abstract_canvas.cpp
    qwt/src/qwt_plot_axis.cpp
    qwt/src/qwt_plot_barchart.cpp
    qwt/src/qwt_plot_canvas.cpp
    qwt/src/qwt_plot_curve.cpp
    qwt/src/qwt_plot_dict.cpp
    qwt/src/qwt_plot_directpainter.cpp
    # qwt/src/qwt_plot_glcanvas.cpp # Not compatible with Qt6
    qwt/src/qwt_plot_graphicitem.cpp
    qwt/src/qwt_plot_grid.cpp
    qwt/src/qwt_plot_histogram.cpp
    qwt/src/qwt_plot_intervalcurve.cpp
    qwt/src/qwt_plot_item.cpp
    qwt/src/qwt_plot_layout.cpp
    qwt/src/qwt_plot_legenditem.cpp
    qwt/src/qwt_plot_magnifier.cpp
    qwt/src/qwt_plot_marker.cpp
    qwt/src/qwt_plot_multi_barchart.cpp
    qwt/src/qwt_plot_opengl_canvas.cpp
    qwt/src/qwt_plot_panner.cpp
    qwt/src/qwt_plot_picker.cpp
    qwt/src/qwt_plot_rasteritem.cpp
    qwt/src/qwt_plot_renderer.cpp
    qwt/src/qwt_plot_rescaler.cpp
    qwt/src/qwt_plot_scaleitem.cpp
    qwt/src/qwt_plot_seriesitem.cpp
    qwt/src/qwt_plot_shapeitem.cpp
    qwt/src/qwt_plot_spectrocurve.cpp
    qwt/src/qwt_plot_spectrogram.cpp
    qwt/src/qwt_plot_svgitem.cpp
    qwt/src/qwt_plot_textlabel.cpp
    qwt/src/qwt_plot_tradingcurve.cpp
    qwt/src/qwt_plot_vectorfield.cpp
    qwt/src/qwt_plot_zoneitem.cpp
    qwt/src/qwt_plot_zoomer.cpp
    qwt/src/qwt_point_3d.cpp
    qwt/src/qwt_point_data.cpp
    qwt/src/qwt_point_mapper.cpp
    qwt/src/qwt_point_polar.cpp
    qwt/src/qwt_polar_canvas.cpp
    qwt/src/qwt_polar_curve.cpp
    qwt/src/qwt_polar_fitter.cpp
    qwt/src/qwt_polar_grid.cpp
    qwt/src/qwt_polar_item.cpp
    qwt/src/qwt_polar_itemdict.cpp
    qwt/src/qwt_polar_layout.cpp
    qwt/src/qwt_polar_magnifier.cpp
    qwt/src/qwt_polar_marker.cpp
    qwt/src/qwt_polar_panner.cpp
    qwt/src/qwt_polar_picker.cpp
    qwt/src/qwt_polar_plot.cpp
    qwt/src/qwt_polar_renderer.cpp
    qwt/src/qwt_polar_spectrogram.cpp
    qwt/src/qwt_raster_data.cpp
    qwt/src/qwt_round_scale_draw.cpp
    qwt/src/qwt_sampling_thread.cpp
    qwt/src/qwt_scale_div.cpp
    qwt/src/qwt_scale_draw.cpp
    qwt/src/qwt_scale_engine.cpp
    qwt/src/qwt_scale_map.cpp
    qwt/src/qwt_scale_widget.cpp
    qwt/src/qwt_series_data.cpp
    qwt/src/qwt_slider.cpp
    qwt/src/qwt_spline.cpp
    qwt/src/qwt_spline_basis.cpp
    qwt/src/qwt_spline_cubic.cpp
    qwt/src/qwt_spline_curve_fitter.cpp
    qwt/src/qwt_spline_local.cpp
    qwt/src/qwt_spline_parametrization.cpp
    qwt/src/qwt_spline_pleasing.cpp
    qwt/src/qwt_spline_polynomial.cpp
    qwt/src/qwt_symbol.cpp
    qwt/src/qwt_system_clock.cpp
    qwt/src/qwt_text.cpp
    qwt/src/qwt_text_engine.cpp
    qwt/src/qwt_text_label.cpp
    qwt/src/qwt_thermo.cpp
    qwt/src/qwt_transform.cpp
    qwt/src/qwt_vectorfield_symbol.cpp
    qwt/src/qwt_weeding_curve_fitter.cpp
    qwt/src/qwt_wheel.cpp
    qwt/src/qwt_widget_overlay.cpp
)

add_library(qwt ${QWT_LIBRARY_TYPE} ${qwt_headers} ${qwt_sources})
add_library(qwt::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(qwt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/qwt/src)
target_link_libraries(qwt
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGL
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::PrintSupport
)
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(qwt PUBLIC Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

# Check if the library is built as a shared library
get_target_property(QWT_LIBRARY_TYPE qwt TYPE)

target_compile_definitions(qwt PRIVATE QWT_MOC_INCLUDE)
if(QWT_LIBRARY_TYPE STREQUAL "SHARED_LIBRARY")
    target_compile_definitions(qwt PUBLIC QWT_DLL)
    target_compile_definitions(qwt PRIVATE QWT_MAKEDLL)
endif()

# Export targets and install them
install(TARGETS qwt
  EXPORT qwtTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT qwt_runtime
)

# For shared libs we need to deploy the Qt DLLs
if(QWT_LIBRARY_TYPE STREQUAL "SHARED_LIBRARY")
    if ((WIN32 OR APPLE) AND (${QT_VERSION_MAJOR} GREATER_EQUAL 6))

        # Generate a script that will deploy all necessary Qt DLLs to the binary folder
        # https://doc.qt.io/qt-6/qt-deploy-runtime-dependencies.html
        # Available for Qt 6.3 and up (=> Not for Qt5!)
        # Executing it requires CMake 3.14 and up, due to policy https://cmake.org/cmake/help/latest/policy/CMP0087.html
        qt_generate_deploy_app_script(
            TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT qt_deploy_script
            NO_COMPILER_RUNTIME
            NO_UNSUPPORTED_PLATFORM_ERROR
        )

        # Add a postbuild script that will also execute the created script via cmake -P
        # This is necessary to make the application startable / debuggable from the build directory.
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DQT_DEPLOY_PREFIX=$<TARGET_FILE_DIR:${PROJECT_NAME}> -DQT_DEPLOY_BIN_DIR=. -P ${qt_deploy_script}
        )

        # Use the script for deploying the qt dlls in the install dir
        install(SCRIPT ${qt_deploy_script})

    elseif(WIN32)

        # For Qt5 we use our legacy script.
        # Deploy Qt DLLs in the binary folder. This is necessary for starting the application from whithin the IDE without having to copy QtCore.dll, QtWidgets.dll etc. by hand each time
        qt_add_windeployqt_postbuild(--no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw --pdb "$<TARGET_FILE:qwt>")

        get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

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
endif()