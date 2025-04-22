#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PORT>"
  echo "Example: $0 /dev/ttyUSB0"
  exit 1
fi

PORT=$1

# Make sure the reduced version is compiled
./compile_reduced.sh

if [ $? -eq 0 ]; then
  # Upload the reduced version
  arduino-cli upload -p $PORT -b esp32:esp32:esp32doit-devkit-v1 --build-path="./build_reduced" ./homato_v1.ino

  if [ $? -eq 0 ]; then
    echo ""
    echo "==================================================="
    echo "Successfully uploaded REDUCED VERSION to $PORT"
    echo "==================================================="
    echo ""
    echo "Your device now has BLE features disabled to save space."
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
fi