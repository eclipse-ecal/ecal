# This base "host" image is a simple Debian-based C++ dev container provided
# as part of the MS VSCode remote dev container project.
# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.159.0/containers/cpp/.devcontainer/base.Dockerfile
ARG VARIANT="buster"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends crossbuild-essential-arm64

# Install the exact protobuf compiler version on the host as the libprotoc we build for target.
RUN git clone https://github.com/protocolbuffers/protobuf.git && \
    cd protobuf && git checkout v3.11.4 && mkdir build && \
    cmake -Bbuild -Hcmake -Dprotobuf_BUILD_TESTS=off && \
    cmake --build build -j 8 --target install
