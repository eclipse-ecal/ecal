@echo off

pushd %~dp0\_build\complete
cmake --build . --config Release
popd

pause