# Homato - Home Automation System

Homato is a complete home automation solution that allows you to control various household devices remotely through a web interface. The system uses MQTT for reliable communication between the control interface and IoT devices, with added Bluetooth connectivity for easy setup and configuration.

## Features

- **Real-time Control**: Toggle multiple home appliances and fixtures including:
  - Main Switch
  - Lights (Regular, Tube, Bed, False Ceiling)
  - Fans
  - Air Conditioner
  - Switch Ports

- **Bluetooth Configuration**: Easy device setup and WiFi configuration via BLE
- **Multiple Network Support**: Store up to 3 WiFi credentials for improved reliability
- **Connection Status Monitoring**: Real-time status indicators for both server and device connectivity
- **Activity Logging**: Track all control operations and status changes
- **State Persistence**: Device states are stored in EEPROM to recover after power outages
- **Mobile-Friendly Interface**: Responsive design that works on all device sizes

## System Architecture

### Components

1. **Web Control Interface**:
   - Node.js backend with Express
   - Socket.IO for real-time communication
   - Clean, responsive UI for device control
   - Mobile app for device setup and control

2. **Firmware**:
   - ESP32-based hardware implementation
   - BLE support for device pairing and configuration
   - Secure MQTT client for cloud communication
   - Multi-network support with fallback capability
   - State persistence via EEPROM

3. **MQTT Broker**:
   - Secure cloud-based message broker (HiveMQ)
   - Topic-based publish/subscribe pattern
   - Supports device status monitoring

## Setup and Installation

### Prerequisites

- Node.js v14+
- Docker (for containerized deployment)
- ESP32 compatible board for hardware
- Mobile device with Bluetooth capability for configuration

### Backend Setup

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/homato.git
   cd homato
   ```

2. Install dependencies:
   ```
   cd homato-control-system
   npm install
   ```

3. Run the application:
   ```
   npm start
   ```

   Or use Docker:
   ```
   sudo docker build -t app .
   sudo docker run -d -it -p 3000:3000 --name appc app
   ```

### Database Setup

1. Install MongoDB:
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt install docker docker-compose

   # macOS with Homebrew
   brew tap mongodb/brew
   brew install docker
   ```

2. Start MongoDB service:
   ```bash
   # to start all the services in compose file.
   docker-compose up -d 

   # to start only the mongodb 
   docker-compose up -d  mongodb
   ```

3. Set environment variables:
   ```bash
   # Add to your .env file
   MONGODB_URI=mongodb://admin:password123@localhost:27000/homato?authSource=admin 
   ```

4. Verify connection:
   ```bash
   # The app should connect automatically on npm start
   npm run dev
   # Or test manually (need to install Mongosh):
   mongosh mongodb://admin:password123@localhost:27000/homato?authSource=admin 
   ```


### Firmware Setup

1. Open the Arduino IDE
2. Install ESP32 board support
3. Open `Firmware/homato_v1/homato_v1.ino`
4. Update MQTT credentials if needed (WiFi can be configured via BLE)
5. Compile and upload to your ESP32 device using the huge_app partition scheme

#### Compilation and Upload Commands

```bash
# Compile with huge_app partition scheme
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 homato_v1.ino

# Upload to the ESP32
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 -p /dev/cu.usbserial-0001 homato_v1.ino
```

## Hardware Setup

### Relay Connections
Connect relays to these GPIO pins on the ESP32:
- Relay 1: GPIO 23
- Relay 2: GPIO 22
- Relay 3: GPIO 21
- Relay 4: GPIO 19
- Relay 5: GPIO 18
- Relay 6: GPIO 5
- Relay 7: GPIO 25
- Relay 8: GPIO 26

### Switch Inputs
Connect switch inputs to these GPIO pins:
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

## Usage

1. Access the web interface at `http://localhost:3000`
2. Use the toggle switches to control your devices
3. Monitor connection status at the top of the interface
4. View activity logs at the bottom of the page

## Security Considerations

- The system uses secure MQTT communication (mqtts)
- For production use, replace `setInsecure()` with proper certificate validation
- Update default credentials before deployment

## License

[MIT License](LICENSE)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgements

- This project uses HiveMQ as the MQTT broker
- Built with Node.js, Express, and Socket.IO
- ESP8266 community for the excellent IoT libraries 


## Logic Flow

```mermaid
graph TD
    subgraph Frontend
        A[Web Interface] -->|User Action| B[Socket.IO Client]
        B -->|Emit Event| C[Backend Server]
    end

    subgraph Backend
        C -->|MQTT Publish| D[MQTT Broker]
        D -->|MQTT Subscribe| C
        C -->|Socket Event| B
    end

    subgraph IoT Device
        E[ESP32] -->|Subscribe| D
        D -->|Publish| E
        E -->|Control| F[Relay Module]
        F -->|Status| E
        E -->|Store State| G[EEPROM]
        G -->|Recover State| E
        E <-->|BLE Pairing| H[Mobile App]
        H -->|WiFi Config| E
    end

    style A fill:#f9f,stroke:#333,stroke-width:2px
    style D fill:#bbf,stroke:#333,stroke-width:2px
    style E fill:#bfb,stroke:#333,stroke-width:2px
```

### Data Flow Description

1. **Device Onboarding**
   - User presses the BLE button on the device for 5 seconds to activate pairing mode
   - Mobile app discovers and connects to the device via BLE
   - Device provides information about its capabilities (relay count, device ID)
   - User configures WiFi credentials through the mobile app
   - Device stores multiple WiFi credentials in EEPROM for fallback

2. **User Interaction**
   - User toggles a device in the web interface
   - Socket.IO client emits control event to backend

3. **Backend Processing**
   - Server receives Socket.IO event
   - Processes request and publishes to MQTT topic
   - Maintains connection status and device states

4. **MQTT Communication**
   - Broker handles message routing between backend and IoT device
   - Manages retained messages and QoS levels
   - Handles device availability status

5. **IoT Device Operation**
   - ESP32 receives MQTT messages
   - Controls appropriate relay
   - Stores state in EEPROM
   - Reports status back through MQTT
   - Handles power recovery and state restoration
   - Automatically tries alternative WiFi networks if primary connection fails

5. **Status Updates**
   - Device publishes state changes
   - Backend subscribes and updates clients
   - Web interface reflects current state
   - Activity log updated

### Error Handling

- Connection loss detection
- Multi-network failover capability
- Automatic reconnection attempts
- BLE mode activation for WiFi reconfiguration
- State persistence during power outages
- Command throttling to prevent relay damage
- Device availability monitoring
- Memory usage optimization to prevent crashes

### Flashing Instructions

- Fully erase flash with 
```bash
python3.11 -m esptool --chip esp32 --port /dev/cu.usbserial-0001 erase_flash
```
- Compile your sketch with 
```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 homato_v1.ino
```
- Upload your fresh firmware with 
```bash
arduino-cli upload --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 -p /dev/cu.usbserial-0001 homato_v1.ino
```

### Mobile App Requirements

The backend team should implement the following in the mobile app:

1. **BLE Discovery**: Scan for BLE devices with service UUID `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
2. **Device Information**: Read device info from characteristic UUID `beb5483e-36e1-4688-b7f5-ea07361b26a8`
3. **WiFi Configuration**: Write WiFi credentials to characteristic UUID `beb5483e-36e1-4688-b7f5-ea07361b26a9` in format `SSID:password`
4. **Feedback Handling**: Process success/error responses from WiFi configuration attempts
5. **Relay Mapping**: UI for mapping relays after successful device onboarding
