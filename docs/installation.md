# Installation Guide

## System Requirements

### Software Requirements
- Node.js v14 or higher
- npm v6 or higher
- Arduino IDE 1.8.x or higher
- Git
- Docker (optional)

### Hardware Requirements
- ESP8266 NodeMCU
- 8-Channel Relay Module (5V)
- 5V/2A Power Supply
- USB Cable for ESP8266
- Jumper Wires
- Terminal Blocks
- Project Enclosure

## Backend Installation

### 1. Clone the Repository
```bash
git clone https://github.com/0xabstracted/homato.git
cd homato
```

### 2. Install Dependencies
```bash
cd homato-control-system
npm install
```

### 3. Environment Setup
```bash
# Copy example environment file
cp .env.example .env

# Edit .env with your settings
PORT=3000
MQTT_HOST=your-mqtt-host
MQTT_PORT=8883
MQTT_USERNAME=your-username
MQTT_PASSWORD=your-password
DEBUG=app:*
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

### 1. Arduino IDE Setup
1. Install Arduino IDE
2. Add ESP8266 Board Support:
   - Open Preferences
   - Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to Additional Boards Manager URLs
   - Install ESP8266 from Boards Manager

### 2. Required Libraries
Install these libraries from Arduino Library Manager:
- PubSubClient
- ESP8266WiFi
- ArduinoJson

### 3. Upload Firmware
1. Open `Firmware/homato_v1/homato_v1.ino`
2. Update WiFi credentials:
   ```cpp
   const char* ssid = "Your_SSID";
   const char* password = "Your_Password";
   ```
3. Update MQTT settings:
   ```cpp
   const char* mqtt_server = "your-mqtt-host";
   const char* mqtt_username = "your-username";
   const char* mqtt_password = "your-password";
   ```
4. Select correct board: Tools → Board → NodeMCU 1.0
5. Select correct port: Tools → Port → COM* or /dev/ttyUSB*
6. Upload firmware

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
sudo docker run -d -it -p 3000:3000 --name appc app

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

2. **ESP8266 Upload Fails**
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
1. Backup current firmware
2. Download new version
3. Update configuration
4. Upload to ESP8266

## Next Steps

1. Read [Hardware Setup](hardware.md) for physical connections
2. Configure [Environment](configuration.md)
3. Review [Security Considerations](security.md)
4. Check [Troubleshooting Guide](troubleshooting.md) if needed 