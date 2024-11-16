@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

cd %BUILD_DIR_COMPLETE%
ctest -C Release --verbose
cd ..

popd
