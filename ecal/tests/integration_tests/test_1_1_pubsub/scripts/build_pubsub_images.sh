#!/bin/bash

# Get directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Example: Path to Dockerfile based on script location
DOCKERFILE_DIR="${SCRIPT_DIR}/../docker"
CONTEXT_DIR="${SCRIPT_DIR}/../"

# Build images
build_image() {
  TAG=$1
  echo "[INFO] Building image: ${TAG}"
  docker build -t ${TAG} -f "${DOCKERFILE_DIR}/Dockerfile" "${CONTEXT_DIR}"
  if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed for ${TAG}!"
    exit 1
  fi
}

build_image ecal_test_pubsub_local_shm
build_image ecal_test_pubsub_local_udp
build_image ecal_test_pubsub_local_tcp
build_image ecal_test_pubsub_network_udp
build_image ecal_test_pubsub_network_tcp

echo "[INFO] All images built successfully!"
