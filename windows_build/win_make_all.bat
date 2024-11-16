@echo off

pushd %~dp0\..

call build_win\win_make_cmake.bat
call build_win\win_make_build.bat
call build_win\win_make_setup.bat
call build_win\win_make_python_wheel.bat

popd
