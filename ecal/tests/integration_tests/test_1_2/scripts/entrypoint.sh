#!/bin/bash
if [ "$1" = "publisher" ]; then
  ./publisher
elif [ "$1" = "subscriber" ]; then
  ./subscriber
else
  echo "Specify either publisher or subscriber"
  exit 1
fi
