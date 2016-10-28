#!/bin/sh

while true; do
  ./usbtemp measure >/dev/null
  sleep 1
  ./usbtemp temp
done
