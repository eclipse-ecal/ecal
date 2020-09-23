@echo off

if ["%~1"]==[""] (
  set VERSION=v142
  ) else (
  set VERSION=%1
  )

rem - base folder
set "WORKSPACE=%~dp0\..\"

rem - cmake will generate the build solution here
set BUILD_DIR_COMPLETE=_build\complete
set BUILD_DIR_SDK=_build\sdk
