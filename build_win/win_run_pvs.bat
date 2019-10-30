@echo off

pushd %~dp0\..

call build_win\win_make_cmake.bat
call build_win\win_make_build.bat

rem - please setup correct pvs installation path here
set PVS_PATH="%PROGRAMFILES%\PVS-Studio"

mkdir "%BUILD_DIR%\_pvs"
""%PVS_PATH%\PVS-Studio_Cmd.exe"" --target "%BUILD_DIR%\eCAL.sln" --platform "x64" --configuration "Release" --output "%BUILD_DIR%\_pvs\ecal.plog" --progress --settings "%WORKSPACE%\PVSSettings.xml"

cd "%BUILD_DIR%\_pvs"
""%PVS_PATH%\PlogConverter.exe ecal.plog -a GA:1,2;OP:1,2 -t FullHtml,Txt,Totals"

popd
