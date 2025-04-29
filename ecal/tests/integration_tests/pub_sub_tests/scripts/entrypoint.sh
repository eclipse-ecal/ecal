#!/bin/bash

ROLE=$1  # publisher, subscriber, both
MODE=$2  # local_shm, local_udp, etc.

if [ -z "$ROLE" ] || [ -z "$MODE" ]; then
  echo "Usage: $0 <publisher|subscriber|both> <mode>"
  exit 1
fi

cd /app/src/build

if [ "$ROLE" = "publisher" ]; then
  echo "[Entrypoint] Starting publisher in mode $MODE"
  ./publisher "$MODE"

elif [ "$ROLE" = "subscriber" ]; then
  echo "[Entrypoint] Starting subscriber in mode $MODE"
  ./subscriber "$MODE"
  sleep 2

elif [ "$ROLE" = "both" ]; then
  echo "[Entrypoint] Starting subscriber and publisher in mode $MODE"
  ./subscriber "$MODE" &
  SUB_PID=$!
  ./publisher "$MODE"
  wait $SUB_PID

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
