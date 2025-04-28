#!/bin/bash

# Get directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Path to Dockerfile and context
DOCKERFILE_DIR="${SCRIPT_DIR}/../docker"
CONTEXT_DIR="${SCRIPT_DIR}/.."

# Base name for image tags
BASE_TAG="ecal_test_pubsub_1_1_2"

# Build function
build_image() {
  TAG=$1
  echo "[INFO] Building image: ${TAG}"
  docker build -t "${TAG}" -f "${DOCKERFILE_DIR}/Dockerfile" "${CONTEXT_DIR}"
  if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed for ${TAG}!"
    exit 1
  fi
}

# Build all needed images
build_image "${BASE_TAG}_local_shm"
build_image "${BASE_TAG}_local_udp"
build_image "${BASE_TAG}_local_tcp"
build_image "${BASE_TAG}_network_udp"
build_image "${BASE_TAG}_network_tcp"

echo "[INFO] All images built successfully!"
