#!/bin/bash

# Create a build directory with the name matching the sketch
mkdir -p build/homato_v1

# Copy the sketch
cp homato_v1.ino build/homato_v1/homato_v1.ino

# Compile with optimizations but including all features
arduino-cli compile \
  --build-property "compiler.cpp.extra_flags=-Os" \
  -b esp32:esp32:esp32doit-devkit-v1 \
  --build-path="./build" \
  ./homato_v1.ino

# Check if it succeeded 
if [ $? -eq 0 ]; then
  echo ""
  echo "==================================================="
  echo "Successfully compiled with ALL FEATURES (including BLE)"
  echo "==================================================="
  echo ""
  echo "This version has all features enabled including BLE."
  echo "The firmware is ready to be uploaded."
  echo ""
  echo "To upload this firmware to your ESP32, use:"
  echo "-----------------------------------------"
  echo "arduino-cli upload -p <YOUR_PORT> -b esp32:esp32:esp32doit-devkit-v1 --build-path=\"./build\" ./homato_v1.ino"
  echo ""
  echo "Or use the simplified upload command:"
  echo "-----------------------------------------"
  echo "./upload_full.sh <YOUR_PORT>"
else
  echo "==================================================="
  echo "Compilation failed"
  echo "==================================================="
  echo "The full version compilation failed, which may be due to size limitations."
  echo "Try compiling the reduced version using ./compile_reduced.sh instead."
fi