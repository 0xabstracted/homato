#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PORT>"
  echo "Example: $0 /dev/ttyUSB0"
  exit 1
fi

PORT=$1

# Make sure the optimized version is compiled
./compile_optimized.sh

if [ $? -eq 0 ]; then
  # Upload the optimized version
  arduino-cli upload -p $PORT -b esp32:esp32:esp32doit-devkit-v1 --build-path="./build_optimized" ./homato_v1.ino

  if [ $? -eq 0 ]; then
    echo ""
    echo "==================================================="
    echo "Successfully uploaded OPTIMIZED VERSION to $PORT"
    echo "==================================================="
    echo ""
    echo "Your device now has BLE and sensor features disabled to save space."
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