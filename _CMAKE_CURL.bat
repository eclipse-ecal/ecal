@echo off

pushd %~dp0

if not exist "_build_curl" mkdir "_build_curl"


pushd %~dp0\_build_curl
cmake ../thirdparty/curl/curl -G "Visual Studio 17 2022" -T v142 -A x64^
		-DCURL_USE_SCHANNEL=ON^
		-DBUILD_CURL_EXE=OFF^
		-DENABLE_MANUAL=OFF^
		-DCMAKE_INSTALL_PREFIX=_install^
		-DBUILD_SHARED_LIBS=OFF
popd

popd

pause