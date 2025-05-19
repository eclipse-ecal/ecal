#!/bin/bash

ROLE=$1        # publisher | subscriber | monitor
MODE=$2        # local_shm | local_udp | etc.
TOPIC=$3       # topic name (pub/sub) OR monitored process (monitor)
NAME=$4        # optional node name
EXTRA=$5       # optional extra args


if [ -z "$ROLE" ] || [ -z "$MODE" ]; then
  echo "Usage: $0 <publisher|subscriber|monitor> <mode> [topic_or_process] [extra args]"
  exit 1
fi

TOPIC=${TOPIC:-test_topic}

cd /app/src/build

if [ "$ROLE" = "crash_publisher" ]; then
  echo "[Entrypoint] Starting crash publisher in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC"
  ./crash_publisher $ARGS $EXTRA

elif [ "$ROLE" = "subscriber" ]; then
  echo "[Entrypoint] Starting crash subscriber in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC"
  ./test_subscriber $ARGS $EXTRA

elif [ "$ROLE" = "monitor" ]; then
  echo "[Entrypoint] Starting monitoring process in mode $MODE"
  ARGS="--mode $MODE"
  ./monitoring $ARGS $EXTRA

elif [ "$ROLE" = "test_publisher" ]; then
  echo "[Entrypoint] Starting resilient publisher in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC --name ${NAME:-test_pub}"
  ./test_publisher $ARGS $EXTRA

elif [ "$ROLE" = "local_all" ]; then
  echo "[Entrypoint] Starting local test (pub/sub/monitor) in mode $MODE"

  ARGS="--mode $MODE --topic $TOPIC"

  ./test_subscriber $ARGS &
  SUB_PID=$!

  ./crash_publisher $ARGS &
  CRASH_PUB_PID=$!

  ./test_publisher $ARGS &
  TEST_PUB_PID=$!

  wait $TEST_PUB_PID
  wait $CRASH_PUB_PID
  wait $SUB_PID

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
