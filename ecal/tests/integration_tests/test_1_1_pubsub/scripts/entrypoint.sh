#!/bin/bash

if [ "$1" = "publisher" ]; then
  ./publisher
elif [ "$1" = "subscriber" ]; then
  ./subscriber
elif [ "$1" = "both" ]; then
  ./subscriber &
  sleep 4
  ./publisher
else
  echo "Specify either publisher, subscriber or both"
  exit 1
fi
