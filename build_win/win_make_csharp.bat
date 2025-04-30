@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

rem -- configure C# binding
if not exist "%WORKSPACE%\%BUILD_DIR_CSHARP%" mkdir "%WORKSPACE%\%BUILD_DIR_CSHARP%"
cd /d "%WORKSPACE%\%BUILD_DIR_CSHARP%"

cmake -S "%WORKSPACE%/lang/csharp" -B "%WORKSPACE%/_build/csharp" ^
      -A x64 ^
      -DBUILD_SHARED_LIBS=OFF ^
      -DCMAKE_PREFIX_PATH="%WORKSPACE%/_install/complete" ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_CONFIGURATION_TYPES=Release ^
      -DECAL_CSHARP_BUILD_SAMPLES=ON ^
      -DECAL_CSHARP_BUILD_TESTS=ON ^
      -DECAL_PROJECT_ROOT="%WORKSPACE%"

cmake --build . --parallel --config Release
cpack -C Release -DCPACK_COMPONENTS_ALL="runtime"

popd
