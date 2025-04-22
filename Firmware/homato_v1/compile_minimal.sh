#!/bin/bash
mkdir -p build
# Create a reduced version of the sketch
cp homato_v1.ino build/
cp custom_partitions.csv build/

echo "Compiling with REDUCED_FEATURES flag to disable BLE functionality"

# Compile with custom partition scheme and REDUCED_FEATURES flag
arduino-cli compile -b esp32:esp32:esp32doit-devkit-v1 \
  --build-property compiler.cpp.extra_flags="-DREDUCED_FEATURES" \
  --build-property build.partitions="custom_partitions.csv" \
  --build-property upload.maximum_size="3145728" \
  ./build/

# If compilation was successful, report the result
if [ $? -eq 0 ]; then
  echo "Compilation successful! The BLE functionality has been disabled to reduce size."
  echo "You can upload this firmware with the following command:"
  echo "arduino-cli upload -p <PORT> -b esp32:esp32:esp32doit-devkit-v1 ./build/homato_v1.ino"
else
  echo "Compilation failed. If you still have size issues, consider:"
  echo "1. Further reducing debug output"
  echo "2. Disabling more features (like sensors)"
  echo "3. Using a larger partition scheme if your ESP32 supports it"
fi