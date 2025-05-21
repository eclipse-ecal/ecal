#!/bin/bash

ROLE=$1        # udp_local_all | udp_network_pub
TOPIC=$2       # optional (default: test_topic)
NAME=$3        # optional container name
EXTRA=$4       # optional extra args

if [ -z "$ROLE" ]; then
  echo "Usage: $0 <udp_local_all|udp_network_pub> [topic] [name] [extra args]"
  exit 1
fi

TOPIC=${TOPIC:-test_topic}
NAME=${NAME:-${ROLE}_node}

cd /app/src/build

if [ "$ROLE" = "udp_local_all" ]; then
  echo "[Entrypoint] Starting UDP subscriber, then UDP publisher..."

  ./network_crash_sub --topic "$TOPIC" --name sub_${NAME} $EXTRA &
  SUB_PID=$!

  ./local_udp_pub --topic "$TOPIC" --name local_udp_pub_${NAME} $EXTRA &
  PUB_PID=$!

  wait $SUB_PID
  EXIT_CODE=$?
  echo "[Entrypoint] Subscriber exited with code $EXIT_CODE"
  exit $EXIT_CODE

elif [ "$ROLE" = "udp_network_pub" ]; then
  echo "[Entrypoint] Starting network UDP publisher..."

  ./network_udp_pub --topic "$TOPIC" --name network_udp_pub_${NAME} $EXTRA
  EXIT_CODE=$?
  echo "[Entrypoint] Network UDP Publisher exited with code $EXIT_CODE"
  exit $EXIT_CODE

else
  echo "[Entrypoint] Unknown role: $ROLE"
  exit 1
fi
