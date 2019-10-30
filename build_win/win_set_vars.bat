@echo off

if ["%~1"]==[""] (
  set VERSION=v140
  ) else (
  set VERSION=%1
  )

rem - base folder
set WORKSPACE=%~dp0\..\

rem - cmake will generate the build solution here
set BUILD_DIR=_build
