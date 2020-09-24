@echo off

pushd "%~dp0\.."

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

echo Downloading NPCAP
powershell -Command "& '%~dp0\download_npcap.ps1'"

set "CMAKE_OPTIONS_COMPLETE=-DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF -DBUILD_DOCS=ON -DECAL_NPCAP_SUPPORT=ON"
set "CMAKE_OPTIONS_SDK=-DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug -DBUILD_DOCS=OFF -DBUILD_APPS=OFF -DBUILD_SAMPLES=OFF -DBUILD_TIME=ON -DBUILD_PY_BINDING=OFF -DBUILD_CSHARP_BINDING=OFF -DBUILD_ECAL_TESTS=OFF -DECAL_LAYER_ICEORYX=OFF -DECAL_INCLUDE_PY_SAMPLES=OFF -DECAL_INSTALL_SAMPLE_SOURCES=OFF -DCPACK_PACK_WITH_INNOSETUP=OFF -DECAL_NPCAP_SUPPORT=ON"

if not exist "%BUILD_DIR_COMPLETE%" mkdir "%BUILD_DIR_COMPLETE%"
if not exist "%BUILD_DIR_SDK%" mkdir "%BUILD_DIR_SDK%"

echo Creating Python venv
if not exist "%BUILD_DIR_COMPLETE%\.venv" mkdir "%BUILD_DIR_COMPLETE%\.venv"
python -m venv "%BUILD_DIR_COMPLETE%\.venv"
CALL "%BUILD_DIR_COMPLETE%\.venv\Scripts\activate.bat"

echo Upgrading pip
python -m pip install --upgrade pip

echo Installing python requirements
pip install wheel
pip install -r requirements.txt

if %ARCH% == "" goto build_no_arch

:build_with_arch

cd /d "%WORKSPACE%\%BUILD_DIR_COMPLETE%"
cmake ../.. -G "%VISUAL_STUDIO_GENERATOR%" -A "%ARCH%" %CMAKE_OPTIONS_COMPLETE%

cd /d "%WORKSPACE%\%BUILD_DIR_SDK%"
cmake ../.. -G "%VISUAL_STUDIO_GENERATOR%" -A "%ARCH%" %CMAKE_OPTIONS_SDK%

goto end

:build_no_arch
cd /d "%WORKSPACE%\%BUILD_DIR_COMPLETE%"
cmake ../.. -G "%VISUAL_STUDIO_GENERATOR% Win64" %CMAKE_OPTIONS_COMPLETE%

cd /d "%WORKSPACE%\%BUILD_DIR_SDK%"
cmake ../.. -G "%VISUAL_STUDIO_GENERATOR% Win64" %CMAKE_OPTIONS_SDK%

:end
popd
