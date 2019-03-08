@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

if ["%~1"]==[""] (
  set VERSION=v140
  ) else (
  set VERSION=%1
  )

set VISUAL_STUDIO_GENERATOR=Visual Studio 14 2015
if %VERSION% == v140 goto build

set VISUAL_STUDIO_GENERATOR=Visual Studio 15 2017
if %VERSION% == v141 goto build

:build
set COMPILE_WIN32=%VISUAL_STUDIO_GENERATOR%
set COMPILE_WIN64=%VISUAL_STUDIO_GENERATOR% Win64

if not exist "%BUILD_DIR%" mkdir %BUILD_DIR%
cd /d %BUILD_DIR%

cmake .. -G "%COMPILE_WIN64%" -DCMAKE_INSTALL_PREFIX=_install

popd
