@echo off

pushd %~dp0\..

call build_win\win_set_vars.bat

cmake --install "%WORKSPACE%\%BUILD_DIR_COMPLETE%" ^
      --prefix "%WORKSPACE%\%INSTALL_DIR_COMPLETE%"

popd
