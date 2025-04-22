#!/bin/bash

# Create a build directory with the name matching the sketch
mkdir -p build_reduced/homato_v1

# Copy the sketch
cp homato_v1.ino build_reduced/homato_v1/homato_v1.ino

# Compile with optimizations and reduced features
arduino-cli compile \
  --build-property "compiler.cpp.extra_flags=-DREDUCED_FEATURES -Os" \
  --build-property build.code_debug=0 \
  -b esp32:esp32:esp32doit-devkit-v1 \
  --build-path="./build_reduced" \
  ./homato_v1.ino

# Check if it succeeded 
if [ $? -eq 0 ]; then
  echo ""
  echo "==================================================="
  echo "Successfully compiled with REDUCED_FEATURES"
  echo "==================================================="
  echo ""
  echo "This version has BLE disabled to reduce code size."
  echo "The firmware is ready to be uploaded."
  echo ""
  echo "IMPORTANT: In this reduced version:"
  echo "- BLE functionality is disabled"
  echo "- Extra debug messages are removed"
  echo "- Code is optimized for size"
  echo ""
  echo "To upload this firmware to your ESP32, use:"
  echo "-----------------------------------------"
  echo "arduino-cli upload -p <YOUR_PORT> -b esp32:esp32:esp32doit-devkit-v1 ./homato_v1.ino"
  echo ""
  echo "To reenable BLE functionality in the future:"
  echo "-------------------------------------------"
  echo "Simply compile the original file normally without using this script."
else
  echo "==================================================="
  echo "Compilation failed"
  echo "==================================================="
  echo "Try these additional steps to further reduce size:"
  echo "1. Disable more features using #ifndef REDUCED_FEATURES"
  echo "2. Remove more debug print statements"
  echo "3. Optimize String usage by avoiding concatenation"
  echo "4. Consider using a different ESP32 board with more flash memory"
fi