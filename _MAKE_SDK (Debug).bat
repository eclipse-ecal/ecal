@echo off

pushd %~dp0\_build\sdk
cmake --build . --config Debug
popd

pause