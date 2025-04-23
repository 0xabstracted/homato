# Installation Guide

This guide provides comprehensive installation instructions for all components of the Homato Home Automation System.

## Prerequisites

Before starting installation, ensure you have the following tools and components:

### Development Environment

- **Node.js** v14 or higher
- **npm** v6 or higher
- **Arduino IDE** 2.0 or higher (or Arduino CLI)
- **Git** for repository cloning
- **Python 3.7+** for esptool and other utilities (optional)

### Hardware Components

- ESP32 Development Board with at least 4MB flash
- 8-Channel Relay Module
- Power Supply (5V/2A)
- DHT11 Temperature/Humidity Sensor
- LDR (Light Dependent Resistor)
- Jumper Wires
- Project Enclosure
- Terminal Blocks
- AC/DC Components as needed
- Mobile Device with Bluetooth for configuration

## Backend Installation

### Setting Up the Node.js Backend

1. Navigate to the backend directory:
   ```bash
   cd homato/homato-control-system
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Create `.env` file:
   ```env
   PORT=3000
   MQTT_HOST=your-mqtt-host
   MQTT_PORT=8883
   MQTT_USERNAME=your-username
   MQTT_PASSWORD=your-password
   # Optional - enable debug logs
   DEBUG=app:*,mqtt:*,socket:*
   ```

### 4. MQTT Broker Setup

#### Using HiveMQ Cloud (Recommended)
1. Create account at [HiveMQ Cloud](https://www.hivemq.com/cloud/)
2. Create a new cluster
3. Get connection details:
   - Host
   - Port (8883 for SSL)
   - Username
   - Password
4. Update `.env` with these details

#### Alternative: Local Mosquitto Broker
```bash
# Install Mosquitto
sudo apt-get install mosquitto mosquitto-clients

# Create config file
sudo nano /etc/mosquitto/conf.d/default.conf

# Add configuration
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd

# Create user
sudo mosquitto_passwd -c /etc/mosquitto/passwd your_username

# Restart Mosquitto
sudo systemctl restart mosquitto
```

## Firmware Installation

### Installing ESP32 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to Additional Board Manager URLs
4. Go to **Tools > Board > Boards Manager**
5. Search for **ESP32** and install the latest version

### Uploading the Firmware

1. Clone the repository:
   ```bash
   git clone https://github.com/0xabstracted/homato.git
   cd homato/Firmware/homato_v1
   ```

2. Open `homato_v1.ino` in Arduino IDE

3. Update MQTT credentials (WiFi will be configured via BLE):
   ```cpp
   const char* mqtt_server = "your-mqtt-broker.com";
   const char* mqtt_username = "your-username";
   const char* mqtt_password = "your-password";
   ```
   
4. Update device identifier if needed:
   ```cpp
   const char* DEVICE_ID = "st-000002"; // Change to a unique ID
   ```

5. Select the correct board, partition scheme, and port:
   - Board: **ESP32 Dev Module**
   - Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**
   - Flash Mode: **QIO**
   - Flash Frequency: **80MHz**
   - Upload Speed: **921600**
   - Port: Select the correct COM port or /dev/ttyUSB*

6. Upload the sketch using the Arduino IDE

   Or use Arduino CLI with the following commands:
   ```bash
   # Compile with huge_app partition scheme
   arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 homato_v1.ino

   # Upload to the ESP32
   arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 -p /dev/cu.usbserial-0001 homato_v1.ino
   ```

7. If you encounter flash errors, you may need to fully erase the flash:
   ```bash
   python3 -m esptool --chip esp32 --port /dev/cu.usbserial-0001 erase_flash
   ```

## Running the Application

### Method 1: Direct Node.js
```bash
# Development mode
npm run dev

# Production mode
npm start
```

### Method 2: Docker (Recommended for Production)
```bash
# Build Docker image
sudo docker build -t app .

# Run container
sudo docker run -d -p 3000:3000 --name appc app

# View logs
sudo docker logs -f appc
```

### Method 3: PM2 Process Manager
```bash
# Install PM2
npm install -g pm2

# Start application
pm2 start ecosystem.config.js

# Monitor application
pm2 monit
```

## BLE Configuration and Device Setup

1. **Enter BLE Pairing Mode**:
   - Power on the ESP32 device
   - Press and hold the BLE button (GPIO 0, usually the BOOT button) for 5 seconds
   - The device LED should start blinking to indicate BLE pairing mode

2. **Configure via Mobile App**:
   - Use the Homato mobile app to discover the device
   - Connect to the device via BLE
   - Follow the app wizard to:  
     a. View device information (ID, relay count)  
     b. Enter WiFi credentials (SSID and password)  
     c. Configure device name and relay labels

3. **Verify Connection**:
   - The device should connect to WiFi after receiving credentials
   - The app will show a success message with the device's IP address
   - The device LED should stop blinking and remain on

## System Verification

1. **Test Backend Connection**:
   - Start the backend: `npm start`
   - Access web interface: http://localhost:3000
   - Verify the interface loads

2. **Test MQTT Connection**:
   - Use an MQTT client like MQTT Explorer
   - Connect to your broker
   - Verify the device publishes to its availability topic (`deviceID/availability`)

3. **Test Device Control**:
   - Try toggling a device in the web interface
   - Verify relay activation
   - Check status feedback

4. **Test Multi-Network Fallback**:
   - If you've configured multiple WiFi networks, test the fallback capability
   - Disable the primary network and verify the device connects to the backup network

## Post-Installation

### 1. Verify Installation
1. Access web interface: `http://localhost:3000`
2. Check device connection status
3. Test each relay control
4. Verify MQTT communication

### 2. Security Setup
1. Enable SSL/TLS:
   ```javascript
   // In app.js
   const options = {
     key: fs.readFileSync('path/to/private.key'),
     cert: fs.readFileSync('path/to/certificate.crt')
   };
   ```
2. Update firmware with SSL certificate:
   ```cpp
   // In homato_v1.ino
   const char* root_ca = \
   "-----BEGIN CERTIFICATE-----\n" \
   "... Your CA Certificate ...\n" \
   "-----END CERTIFICATE-----\n";
   ```

### 3. Backup Configuration
1. Save `.env` file
2. Export MQTT broker settings
3. Backup firmware code
4. Document hardware connections

## Troubleshooting Installation

### Common Issues

1. **npm Install Fails**
   ```bash
   # Clear npm cache
   npm cache clean --force
   # Try installation with verbose logging
   npm install --verbose
   ```

2. **ESP32 Upload Fails**
   - Check USB connection
   - Verify correct COM port
   - Hold FLASH button while uploading
   - Try different USB cable

3. **MQTT Connection Issues**
   - Verify broker credentials
   - Check network firewall
   - Test with mosquitto_pub/sub
   - Enable debug logging

## Updating

### Backend Updates
```bash
# Pull latest changes
git pull origin main

# Install dependencies
npm install

# Rebuild (if needed)
npm run build
```

### Firmware Updates
1. Pull the latest code:
   ```bash
   cd homato
   git pull
   ```

2. Open the updated firmware in Arduino IDE
3. Ensure the partition scheme is still set to huge_app
4. Upload following the same process as initial installation

### Preserving WiFi Configuration

When updating firmware, the WiFi credentials stored in EEPROM will be preserved unless:
1. You perform a full flash erase
2. The firmware update includes changes to the EEPROM storage structure

If WiFi credentials are lost during update, you can easily reconfigure via BLE pairing.

## Next Steps

1. Read [Hardware Setup](hardware.md) for physical connections
2. Configure [Environment](configuration.md)
3. Review [Security Considerations](security.md)
4. Check [Troubleshooting Guide](troubleshooting.md) if needed