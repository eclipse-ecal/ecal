# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================


set (qt_windeployqt_cmake_path ${CMAKE_CURRENT_LIST_DIR})

# Create convenient function to run windeployqt
function(qt_add_windeployqt_postbuild arguments)
    # Declare windeployqt as executable target
    if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
        get_target_property(_qt5_qmake_location
            Qt5::qmake IMPORTED_LOCATION
        )

        execute_process(
            COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt5_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

        if(EXISTS ${imported_location})
            add_executable(Qt5::windeployqt IMPORTED)

            set_target_properties(Qt5::windeployqt PROPERTIES
                IMPORTED_LOCATION ${imported_location}
            )
        endif()
    endif()

    if(TARGET Qt5::windeployqt)
        configure_file(${qt_windeployqt_cmake_path}/qt_windeployqt_threadsafe_cmake.bat.in qt_windeployqt_threadsafe_cmake.bat
            NEWLINE_STYLE DOS
        )
        
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND call ${CMAKE_CURRENT_BINARY_DIR}/qt_windeployqt_threadsafe_cmake.bat ${ARGV}
        )
    endif()
endfunction(qt_add_windeployqt_postbuild)
