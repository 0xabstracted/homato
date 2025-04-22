#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <PORT>"
  echo "Example: $0 /dev/ttyUSB0"
  exit 1
fi

PORT=$1

echo "====================================================="
echo "Homato Smart Upload Script"
echo "====================================================="
echo "This script will automatically choose the right version"
echo "based on your ESP32's flash capacity."
echo ""

# First try the reduced version (BLE disabled but sensors enabled)
echo "Attempting to compile reduced version (BLE disabled, sensors enabled)..."

./compile_reduced.sh > /tmp/compile_reduced_output.txt 2>&1
  
if [ $? -eq 0 ]; then
  echo "✅ Reduced version compilation successful!"
  echo "Uploading version with BLE disabled but sensors enabled..."
  
  arduino-cli upload -p $PORT -b esp32:esp32:esp32doit-devkit-v1 --build-path="./build_reduced" ./homato_v1.ino
  
  if [ $? -eq 0 ]; then
    echo ""
    echo "====================================================="
    echo "✅ Successfully uploaded REDUCED VERSION to $PORT"
    echo "====================================================="
    echo "Your device now has sensors enabled but BLE disabled to save space."
    exit 0
  else
    echo ""
    echo "❌ Upload failed. Trying with optimized version..."
  fi
else
  echo "❌ Reduced version compilation failed due to size constraints."
  echo "Trying fully optimized version with both BLE and sensors disabled..."
fi

# Try the fully optimized version (both BLE and sensors disabled)
./compile_optimized.sh > /tmp/compile_optimized_output.txt 2>&1

if [ $? -eq 0 ]; then
  echo "✅ Optimized version compilation successful!"
  echo "Uploading optimized version (no BLE, no sensors)..."
  
  arduino-cli upload -p $PORT -b esp32:esp32:esp32doit-devkit-v1 --build-path="./build_optimized" ./homato_v1.ino
  
  if [ $? -eq 0 ]; then
    echo ""
    echo "====================================================="
    echo "✅ Successfully uploaded OPTIMIZED VERSION to $PORT"
    echo "====================================================="
    echo "Your device now has both BLE and sensor features disabled to save space."
    exit 0
  else
    echo ""
    echo "❌ All upload attempts failed."
    echo "====================================================="
    echo "Please check:"
    echo "1. The device is connected to port $PORT"
    echo "2. You have permission to access the port"
    echo "3. The ESP32 is in upload mode (hold BOOT button during reset if needed)"
    exit 1
  fi
else
  echo "❌ All compilation attempts failed."
  echo "====================================================="
  echo "Please verify your Arduino environment setup and board selection."
  echo "If problems persist, you may need a different ESP32 board with more memory."
  exit 1
fi