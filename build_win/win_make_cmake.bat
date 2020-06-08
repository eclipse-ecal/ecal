@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

if ["%~1"]==[""] (
  set VERSION=v142
  ) else (
  set VERSION=%1
  )

set VISUAL_STUDIO_GENERATOR=Visual Studio 14 2015
set ARCH=""
if %VERSION% == v140 goto build

set VISUAL_STUDIO_GENERATOR=Visual Studio 15 2017
set ARCH=""
if %VERSION% == v141 goto build

set VISUAL_STUDIO_GENERATOR=Visual Studio 16 2019
set ARCH=x64
if %VERSION% == v142 goto build

:build
if not exist "%BUILD_DIR%" mkdir %BUILD_DIR%
cd /d %BUILD_DIR%

if %ARCH% == "" goto build_no_arch
set COMPILE_WIN64=%VISUAL_STUDIO_GENERATOR%
cmake .. -G "%COMPILE_WIN64%" -A "%ARCH%" -DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF
goto end

:build_no_arch
set COMPILE_WIN64=%VISUAL_STUDIO_GENERATOR% Win64
cmake .. -G "%COMPILE_WIN64%" -DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF

:end
popd
