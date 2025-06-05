#!/bin/bash

# Usage: ./build_images.sh <tag_prefix>
# Example: ./build_images.sh multi_pub_sub

TAG_PREFIX="$1"

if [ -z "$TAG_PREFIX" ]; then
  echo "[ERROR] Usage: $0 <tag_prefix>"
  exit 1
fi

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CASE_DIR="${SCRIPT_DIR}/.."
DOCKERFILE_DIR="${CASE_DIR}/docker"
CONTEXT_DIR="${CASE_DIR}"

# Build function
build_image() {
  TAG=$1
  echo "[INFO] Building image: ${TAG}"
  docker build -t "${TAG}" -f "${DOCKERFILE_DIR}/Dockerfile" "${BASE_DIR}"
  if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed for ${TAG}!"
    exit 1
  fi
}

# Build all variants
build_image "${TAG_PREFIX}_local_shm"
build_image "${TAG_PREFIX}_local_udp"
build_image "${TAG_PREFIX}_local_tcp"
build_image "${TAG_PREFIX}_network_udp"
build_image "${TAG_PREFIX}_network_tcp"

echo "[✓] All images built successfully!"
