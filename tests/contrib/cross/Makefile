setup_qemu:
	docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

build-target-sysroot: setup_qemu
	docker build -t sysroot-example:ubuntu18 -f ./target/ubuntu18.Dockerfile --build-arg BASEIMAGE=arm64v8/ubuntu:18.04 .
	docker run --rm -d --entrypoint /bin/bash --name sysroot-example sysroot-example:ubuntu18 -c 'while sleep 3600; do :; done'
	mkdir -p ./artifacts/sysroot
	docker export sysroot-example > ./artifacts/sysroot.tar
	docker stop sysroot-example
	tar xf ./artifacts/sysroot.tar -C ./artifacts/sysroot/

build-host:
	docker build -f ./host/Dockerfile -t ecal-crossbuild-host:latest .

run-host: build-host
	docker run --rm -it \
		-u $(shell id -u):$(shell id -u) \
		-v $(PWD)/../../..:/workspaces/ecal \
		-w /workspaces/ecal/tests/contrib/cross \
		ecal-crossbuild-host \
		/bin/bash

clean-target:
	rm -rf artifacts

clean:
	rm -rf build

prebuild:
	mkdir -p build

ecal: prebuild
	cmake -Bbuild -H../../../ \
		-DCMAKE_TOOLCHAIN_FILE=$(PWD)/toolchain-arm64.cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DECAL_THIRDPARTY_BUILD_PROTOBUF=ON \
		-DECAL_THIRDPARTY_BUILD_CURL=OFF \
		-DECAL_THIRDPARTY_BUILD_HDF5=OFF \
		-DECAL_THIRDPARTY_BUILD_CURL=ON \
		-DECAL_USE_QT=OFF
	
	cmake --build build -j $(shell nproc)
