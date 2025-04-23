# Firmware Documentation

This document provides comprehensive information about the Homato firmware, including setup instructions, configuration options, and recent updates.

## Firmware Overview

The Homato firmware is built for ESP32 microcontrollers and provides:
- Control of up to 8 relay channels
- Secure MQTT communication
- WiFi connectivity with multi-network support
- Bluetooth Low Energy (BLE) for device pairing and configuration
- State persistence using EEPROM
- Temperature and humidity monitoring via DHT sensor
- Light level monitoring via LDR

## ESP32 Requirements

The firmware requires an ESP32 development board with:
- At least 4MB of flash memory
- Built-in BLE support
- Sufficient GPIO pins for relays and sensors

## Compilation and Flashing

### Partition Scheme

The firmware requires the **huge_app** partition scheme to accommodate all features. This provides 3MB application space while still reserving 1MB for the SPIFFS file system.

### Compilation Command

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 homato_v1.ino
```

### Upload Command

```bash
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 -p /dev/cu.usbserial-0001 homato_v1.ino
```

### Full Flash Erase (if needed)

```bash
python3.11 -m esptool --chip esp32 --port /dev/cu.usbserial-0001 erase_flash
```

## Memory Optimization Techniques

The firmware has been optimized for memory usage through:

1. **Reduced Serial Output**: Serial printing has been minimized to essential information
2. **Simplified WiFi Connection Logic**: The connection process has been streamlined
3. **Conditional Compilation**: Some features can be disabled to save space
4. **Optimized String Usage**: Minimized heap fragmentation from string operations
5. **EEPROM Usage Reduction**: Only essential data is stored in EEPROM

## Pin Configuration

The following GPIO pins are used in the firmware:

### Relay Outputs
- Relay 1: GPIO 23
- Relay 2: GPIO 22
- Relay 3: GPIO 21
- Relay 4: GPIO 19
- Relay 5: GPIO 18
- Relay 6: GPIO 5
- Relay 7: GPIO 25
- Relay 8: GPIO 26

### Switch Inputs
- Switch 1: GPIO 13
- Switch 2: GPIO 12
- Switch 3: GPIO 14
- Switch 4: GPIO 27
- Switch 5: GPIO 33
- Switch 6: GPIO 32
- Switch 7: GPIO 15
- Switch 8: GPIO 4

### Other Components
- BLE Button: GPIO 0 (usually the BOOT button on ESP32)
- WiFi Status LED: GPIO 2 (built-in LED on most ESP32 boards)
- DHT11 Temperature Sensor: GPIO 16
- LDR (Light Sensor): GPIO 34

## BLE Configuration

The firmware includes BLE functionality for easy device setup:

1. **Activation**: Long-press the BLE button (GPIO 0) for 5 seconds
2. **Pairing**: Device becomes discoverable for BLE pairing
3. **Device Information**: Advertises device ID and capabilities
4. **WiFi Configuration**: Accepts WiFi credentials via BLE
5. **Storage**: Stores up to 3 WiFi network credentials

### BLE Service and Characteristic UUIDs

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Device Info Characteristic**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **WiFi Config Characteristic**: `beb5483e-36e1-4688-b7f5-ea07361b26a9`

## WiFi Configuration

The firmware can store up to 3 different WiFi network credentials:

1. **Storage**: Credentials are stored in EEPROM
2. **Failover**: If primary network is unavailable, it tries alternative networks
3. **Update**: New credentials can be added via BLE
4. **Structure**: Each credential includes SSID, password, and valid flag

## MQTT Configuration

Configure MQTT settings in the firmware:

```cpp
const char* mqtt_server = "your-mqtt-broker.com";
const char* mqtt_username = "your-username";
const char* mqtt_password = "your-password";
const int mqtt_port = 8883;
```

The MQTT topics are automatically generated based on the device ID:

```cpp
char mqtt_topic_relay1[50];  // Format: deviceID/relay1
char mqtt_topic_relay2[50];  // Format: deviceID/relay2
// ... and so on for other relays
char mqtt_topic_availability[50];  // Format: deviceID/availability
```

## State Persistence

Device states are stored in EEPROM to survive power cycles:

1. **Storage Addresses**: Each relay state has a dedicated EEPROM address
2. **Write Timing**: States are written immediately upon change
3. **Recovery**: States are restored during device boot
4. **WiFi Credentials**: Stored in a different section of EEPROM

## Heartbeat Functionality

The device sends regular heartbeat messages to indicate its online status:

1. **Interval**: Heartbeat sent every 30 seconds
2. **Topic**: Published to `deviceID/availability`
3. **LWT (Last Will and Testament)**: Automatically sends "offline" if disconnected

## Debugging

For debugging purposes:

1. **Serial Monitor**: Connect at 115200 baud rate
2. **Status Messages**: Device outputs key events to serial
3. **LED Indicators**: Built-in LED shows WiFi and BLE status
4. **Error Codes**: MQTT connection errors are reported with codes

## Firmware Troubleshooting

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| Compilation fails | Memory overflow | Ensure huge_app partition scheme is selected |
| BLE not working | BLE libraries conflict | Verify BLE includes are correct |
| WiFi not connecting | Incorrect credentials | Use BLE to reconfigure WiFi |
| Relays not responding | Incorrect pin assignment | Verify GPIO pin configuration |
| MQTT disconnections | Network issues | Check broker settings and connection |

## Recent Updates

1. **Migrated to ESP32**: Updated from ESP8266 to ESP32 for BLE support
2. **Added BLE Functionality**: For device pairing and configuration
3. **Multi-Network Support**: Store up to 3 WiFi credentials
4. **Memory Optimization**: Reduced memory usage for stability
5. **Partition Scheme**: Using huge_app for increased application space
