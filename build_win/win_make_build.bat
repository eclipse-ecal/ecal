@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

cmake --build "%BUILD_DIR%" --parallel --config Release
cmake --build "%BUILD_DIR%" --target documentation_c
cmake --build "%BUILD_DIR%" --target documentation_cpp
cmake --build "%BUILD_DIR%" --target install --config Release

popd
