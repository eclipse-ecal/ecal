#!/bin/bash

ROLE=$1        # large_publisher | crash_subscriber | test_subscriber | local_all
MODE=$2        # local_shm | local_udp | network_udp | etc.
TOPIC=$3       # Topic name
NAME=$4        # Optional name (not used)
EXTRA=$5       # Optional extra args

if [ -z "$ROLE" ] || [ -z "$MODE" ]; then
  echo "Usage: $0 <large_publisher|crash_subscriber|test_subscriber|local_all> <mode> [topic] [extra args]"
  exit 1
fi

TOPIC=${TOPIC:-test_topic}
cd /app/src/build

ARGS="--mode $MODE --topic $TOPIC"

if [ "$ROLE" = "large_publisher" ]; then
  echo "[Entrypoint] Starting large publisher in mode $MODE"
  ./large_publisher $ARGS $EXTRA

elif [ "$ROLE" = "test_subscriber" ]; then
  echo "[Entrypoint] Starting stable subscriber in mode $MODE"
  ./test_subscriber $ARGS $EXTRA
  sleep 1
elif [ "$ROLE" = "crash_subscriber" ]; then
  echo "[Entrypoint] Starting crashing subscriber in mode $MODE"
  ./crash_send_subscriber $ARGS $EXTRA
  sleep 1
elif [ "$ROLE" = "local_all" ]; then
  echo "[Entrypoint] Starting all processes locally in one container (mode $MODE)"
  ./large_publisher $ARGS &
  PUB_PID=$!
  sleep 1
  ./test_subscriber $ARGS &
  STABLE_PID=$!
  ./crash_send_subscriber $ARGS &
  CRASH_PID=$!

  wait $STABLE_PID
  
elif [ "$ROLE" = "zero_copy_local_all" ]; then
  echo "[Entrypoint] Starting all processes with Zero-Copy Publisher in one container (mode $MODE)"
  ./zero_copy_pub $ARGS &
  PUB_PID=$!
  sleep 1
  ./test_subscriber $ARGS &
  STABLE_PID=$!
  ./crash_send_subscriber $ARGS &
  CRASH_PID=$!

  wait $STABLE_PID

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
