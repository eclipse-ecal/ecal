@echo OFF

pushd %~dp0
call build_win\win_make_build.bat
popd

pause