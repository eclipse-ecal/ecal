@echo OFF

pushd %~dp0\_build\sdk
cpack -C Debug
popd

pushd %~dp0\_build\complete
cpack -C Release
popd

pause