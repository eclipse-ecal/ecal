@echo OFF

pushd %~dp0
call build_win\win_make_setup.bat
popd

pause