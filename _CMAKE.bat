@echo off

pushd %~dp0

rem set "QT5_ROOT_DIRECTORY=D:\Programme\Qt\5.11.1"
rem set "QT5_ROOT_DIRECTORY=C:\Qt\5.15.2"

rem set "CMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2015_64"

REM call build_win\win_make_cmake.bat v140
REM call build_win\win_make_cmake.bat v142
call build_win\win_make_cmake.bat -T v140

rem for %%I in (.) do set CurrDirName=%%~nxI
rem copy _build\complete_x64\eCAL.sln _build\complete_x64\%CurrDirName%.sln /Y

popd

pause