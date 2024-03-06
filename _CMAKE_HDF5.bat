@echo off

pushd %~dp0

if not exist "_build_hdf5" mkdir "_build_hdf5"


pushd %~dp0\_build_hdf5
cmake ../thirdparty/hdf5/hdf5 -G "Visual Studio 17 2022" -T v142 -A x64^
		-DHDF5_GENERATE_HEADERS=ON^
		-DONLY_SHARED_LIBS=ON^
		-DHDF5_ENABLE_THREADSAFE=ON^
		-DBUILD_TESTING=OFF^
		-DHDF5_BUILD_UTILS=OFF^
		-DHDF5_BUILD_TOOLS=OFF^
		-DHDF5_BUILD_EXAMPLES=OFF^
		-DHDF5_BUILD_CPP_LIB=OFF^
		-DHDF5_BUILD_HL_LIB=OFF^
		-DCMAKE_INSTALL_PREFIX=_install^
		-DBUILD_SHARED_LIBS=ON
popd

popd

pause