@echo OFF

echo Press Enter to clean _build directory

pause

pushd %~dp0
RMDIR /Q/S _build
popd