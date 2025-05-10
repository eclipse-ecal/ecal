#!/bin/bash

ROLE=$1        # publisher | subscriber | both
MODE=$2        # local_shm | local_udp | etc.
TOPIC=$3       # optional topic name
NAME=$4        # optional node name
EXTRA=$5       # optional extra args

if [ -z "$ROLE" ] || [ -z "$MODE" ]; then
  echo "Usage: $0 <publisher|subscriber|both> <mode> [topic] [name] [extra args]"
  exit 1
fi

TOPIC=${TOPIC:-test_topic}
NAME=${NAME:-${ROLE}_test}

cd /app/src/build

ARGS="--mode $MODE --topic $TOPIC --name $NAME"

if [ "$ROLE" = "publisher" ]; then
  echo "[Entrypoint] Starting publisher in mode $MODE"
  ./multi_publisher $ARGS $EXTRA

elif [ "$ROLE" = "subscriber" ]; then
  echo "[Entrypoint] Starting subscriber in mode $MODE"
  ./multi_subscriber $ARGS $EXTRA
  sleep 2

elif [ "$ROLE" = "both" ]; then
  echo "[Entrypoint] Starting subscriber and publisher in mode $MODE"
  ./multi_subscriber -m "$MODE" &
  sleep 1
  SUB_PID=$!
  ./multi_publisher -m "$MODE"
  wait $SUB_PID

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
