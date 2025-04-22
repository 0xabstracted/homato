#!/bin/bash

# Create a build directory with the name matching the sketch
mkdir -p build_optimized/homato_v1

# Copy the sketch
cp homato_v1.ino build_optimized/homato_v1/homato_v1.ino

# Compile with optimizations but configure sensor flags
arduino-cli compile \
  --build-property "compiler.cpp.extra_flags=-DREDUCED_FEATURES -DENABLE_SENSORS=0 -Os" \
  --build-property build.code_debug=0 \
  -b esp32:esp32:esp32doit-devkit-v1 \
  --build-path="./build_optimized" \
  ./homato_v1.ino

# Check if it succeeded 
if [ $? -eq 0 ]; then
  echo ""
  echo "==================================================="
  echo "Successfully compiled OPTIMIZED version"
  echo "==================================================="
  echo ""
  echo "This version has BOTH BLE and SENSORS disabled for maximum code size reduction."
  echo "The firmware is ready to be uploaded."
  echo ""
  echo "IMPORTANT: In this optimized version:"
  echo "- BLE functionality is disabled"
  echo "- Sensor functionality is disabled"
  echo "- Extra debug messages are removed"
  echo "- Code is optimized for size"
  echo ""
  echo "To upload this firmware to your ESP32, use:"
  echo "-----------------------------------------"
  echo "./upload_optimized.sh <YOUR_PORT>"
  echo ""
  echo "To reenable all functionality in the future:"
  echo "-------------------------------------------"
  echo "Simply compile the original file using ./compile_full.sh"
else
  echo "==================================================="
  echo "Compilation failed"
  echo "==================================================="
  echo "Try these additional steps to further reduce size:"
  echo "1. Disable more features using conditional compilation"
  echo "2. Remove more debug print statements"
  echo "3. Consider using a different ESP32 board with more flash memory"
fi