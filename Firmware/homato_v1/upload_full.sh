#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PORT>"
  echo "Example: $0 /dev/ttyUSB0"
  exit 1
fi

PORT=$1

# Make sure the full version is compiled
./compile_full.sh

if [ $? -eq 0 ]; then
  # Upload the full version
  arduino-cli upload -p $PORT -b esp32:esp32:esp32doit-devkit-v1 --build-path="./build" ./homato_v1.ino

  if [ $? -eq 0 ]; then
    echo ""
    echo "==================================================="
    echo "Successfully uploaded FULL VERSION to $PORT"
    echo "==================================================="
    echo ""
    echo "Your device now has all features enabled including BLE."
  else
    echo ""
    echo "==================================================="
    echo "Upload failed"
    echo "==================================================="
    echo "Check if:"
    echo "1. The device is connected to port $PORT"
    echo "2. You have permission to access the port"
    echo "3. The ESP32 is in upload mode (hold BOOT button during reset if needed)"
  fi
else
  echo ""
  echo "==================================================="
  echo "Compilation failed - cannot upload"
  echo "==================================================="
  echo "The full version is too large for your ESP32. Try a reduced version instead."
  echo "Run: ./upload_reduced.sh $PORT"
fi