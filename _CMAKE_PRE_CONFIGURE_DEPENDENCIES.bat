@echo off

pushd %~dp0

if not exist "_build\pre_config_deps" mkdir "_build\pre_config_deps"

pushd %~dp0\_build\pre_config_deps
cmake ../.. -G "Visual Studio 17 2022" -A x64 -T v142 ^
        -DECAL_THIRDPARTY_BUILD_CURL=ON ^
        -DECAL_THIRDPARTY_BUILD_HDF5=ON ^
        -DBUILD_SHARED_LIBS=OFF
popd 

popd

pause