@echo off

rem - base folder
set "WORKSPACE=%~dp0\..\"

rem - cmake will generate the build solution here
set BUILD_DIR_COMPLETE=_build\complete
set BUILD_DIR_SDK=_build\sdk

rem -- install dir for C# to pick up
set INSTALL_DIR_COMPLETE=_install\complete

rem -- C# build directory
set BUILD_DIR_CSHARP=_build\csharp
