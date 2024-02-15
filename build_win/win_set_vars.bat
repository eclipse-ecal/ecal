@echo off

rem - base folder
set "WORKSPACE=%~dp0\..\"

rem - cmake will generate the build solution here
set BUILD_DIR_COMPLETE=_build\complete
set BUILD_DIR_SDK=_build\sdk

rem replace with your Qt installation path:
set CMAKE_PREFIX_PATH=C:\Qt\6.6.2\msvc2019_64
