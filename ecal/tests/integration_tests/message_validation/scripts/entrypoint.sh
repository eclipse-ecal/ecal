#!/bin/bash

ROLE=$1        # publisher | subscriber
MODE=$2        # local_shm | local_udp | etc.
TOPIC=$3       # optional topic name
NAME=$4        # optional node name
EXTRA=$5       # optional extra args



cd /app/src/build

if [ "$ROLE" = "publisher" ]; then
  if [[ "$EXTRA" == *"--malformed"* ]]; then
    echo "[Entrypoint] Sending malformed message."
    ./message_publisher -m "$MODE" -t "$TOPIC" -n "$NAME" --malformed
    sleep 1
  else
    ./message_publisher -m "$MODE" -t "$TOPIC" -n "$NAME"
    sleep 1
  fi

elif [ "$ROLE" = "subscriber" ]; then
  ./message_subscriber -m "$MODE" -t "$TOPIC" -n "$NAME"
  sleep 3

elif [ "$ROLE" = "both" ]; then
  ./message_subscriber -m "$MODE" -t "$TOPIC" -n "${NAME}_sub" &
  sleep 3
  
  if [[ "$EXTRA" == *"--malformed"* ]]; then
    echo "[Entrypoint] Sending malformed message."
    ./message_publisher -m "$MODE" -t "$TOPIC" -n "${NAME}_pub" --malformed
    sleep 1
  else
    ./message_publisher -m "$MODE" -t "$TOPIC" -n "${NAME}_pub"
    sleep 1
  fi
else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
