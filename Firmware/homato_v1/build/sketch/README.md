#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/README.md"
# Homato Firmware v1

## Overview
This is the firmware for the Homato IoT device, which controls relays, monitors sensors, and communicates via MQTT and BLE.

## Size Optimization
The firmware has been optimized to handle the ESP32's flash size constraints. Three compilation options are available:

### Full Version (Standard)
- Complete functionality including WiFi, MQTT, sensors, and BLE
- Allows configuration via mobile app over Bluetooth
- All relay control and monitoring features
- May exceed program storage space on some ESP32 boards

### Reduced Version
- Core functionality: WiFi, MQTT, relay control, sensors
- BLE functionality disabled to reduce code size
- Optimized for smaller ESP32 boards
- Approximately 78% of program storage space used (vs 135% in full version)

### Optimized Version
- Minimal functionality: WiFi, MQTT, relay control
- Both BLE and sensor functionality disabled
- Maximum code size reduction
- For use on boards with severe memory constraints

## Compilation Options

### Using Smart Script (Recommended)
```bash
# Automatically tries each version in order until one succeeds
./upload_smart.sh <PORT>
```

### Standard Compilation
```bash
# Regular compilation (with all features)
./compile_full.sh

# Upload to device
./upload_full.sh <PORT>
```

### Reduced-Size Compilation
```bash
# Compile with BLE disabled but sensors enabled
./compile_reduced.sh

# Upload to device
./upload_reduced.sh <PORT>
```

### Optimized Compilation
```bash
# Compile with both BLE and sensors disabled
./compile_optimized.sh

# Upload to device
./upload_optimized.sh <PORT>
```

## Usage Notes

- The reduced version disables Bluetooth configuration. Initial WiFi setup must be done via hardcoded credentials.
- The optimized version disables both Bluetooth and sensors for maximum memory savings.
- When BLE functionality is disabled, the pairing button will have no effect.
- All core relay control functionality remains intact in all versions.

## Manual Compilation
If you prefer using arduino-cli directly:

```bash
# Full version
arduino-cli compile -b esp32:esp32:esp32doit-devkit-v1 ./homato_v1.ino

# Reduced version (no BLE)
arduino-cli compile --build-property "compiler.cpp.extra_flags=-DREDUCED_FEATURES -Os" -b esp32:esp32:esp32doit-devkit-v1 ./homato_v1.ino

# Optimized version (no BLE, no sensors)
arduino-cli compile --build-property "compiler.cpp.extra_flags=-DREDUCED_FEATURES -DENABLE_SENSORS=0 -Os" -b esp32:esp32:esp32doit-devkit-v1 ./homato_v1.ino
```