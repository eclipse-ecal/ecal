#!/bin/bash

ROLE=$1        # publisher | subscriber | monitor
MODE=$2        # local_shm | local_udp | etc.
TOPIC=$3       # topic name (pub/sub) OR monitored process (monitor)
EXTRA=$4       # optional extra args

if [ -z "$ROLE" ] || [ -z "$MODE" ]; then
  echo "Usage: $0 <publisher|subscriber|monitor> <mode> [topic_or_process] [extra args]"
  exit 1
fi

TOPIC=${TOPIC:-test_topic}

cd /app/src/build

if [ "$ROLE" = "publisher" ]; then
  echo "[Entrypoint] Starting crash publisher in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC"
  ./crash_publisher $ARGS $EXTRA

elif [ "$ROLE" = "subscriber" ]; then
  echo "[Entrypoint] Starting crash subscriber in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC"
  ./crash_subscriber $ARGS $EXTRA

elif [ "$ROLE" = "monitor" ]; then
  echo "[Entrypoint] Starting monitoring process in mode $MODE"
  ARGS="--mode $MODE"
  ./monitoring $ARGS $EXTRA
  sleep 1

elif [ "$ROLE" = "test_pub" ]; then
  echo "[Entrypoint] Starting resilient publisher in mode $MODE"
  ARGS="--mode $MODE --topic $TOPIC --name ${NAME:-test_pub}"
  ./test_publisher $ARGS $EXTRA

elif [ "$ROLE" = "local_all" ]; then
  echo "[Entrypoint] Starting local test (pub/sub/monitor) in mode $MODE"

  ARGS="--mode $MODE --topic $TOPIC"

  ./monitoring "--mode $MODE" &
  MON_PID=$!

  sleep 1

  ./crash_subscriber $ARGS &
  SUB_PID=$!

  ./crash_publisher $ARGS &
  PUB_PID=$!

  wait $SUB_PID
  wait $PUB_PID
  wait $MON_PID

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
