ARG BASEIMAGE=ubuntu/bionic
FROM ${BASEIMAGE}

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
ENV DEBIAN_FRONTEND noninteractive

# This replicates having a sysroot available with installed/prebuilt dependencies
# listed below. This may come from user's build system or provided by third party prior to reaching this point.
# Different configurations can be tested by changing this dependency list.
# Here we have protobuf, hdf5, and libssl (but not curl)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libhdf5-dev \
    libssl-dev \
    zlib1g-dev