@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

cd /d %BUILD_DIR_COMPLETE%

python -m build

popd
