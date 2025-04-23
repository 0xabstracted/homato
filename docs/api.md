# API Documentation

This document details the API interfaces for the Homato Home Automation System, including MQTT topics for device communication and BLE services for device configuration.

## Overview

The Homato API consists of three main components:
- REST API endpoints
- Socket.IO events
- MQTT topics

## REST API

### Base URL
```
http://localhost:3000/api
```

### Endpoints

#### Get Device Status
```http
GET /status
```

**Response**
```json
{
  "deviceState": {
    "switch": "OFF",
    "light": "ON",
    "fan": "OFF",
    "tubelight": "OFF",
    "bedlight": "ON",
    "falseceiling": "OFF",
    "ac": "OFF",
    "switchport": "OFF",
    "deviceConnected": true
  }
}
```

## Socket.IO Events

### Client to Server Events

#### Control Device
```javascript
socket.emit('controlDevice', {
  device: 'light',  // Device identifier
  state: 'ON'      // 'ON' or 'OFF'
});
```

**Available Relays:**
- `relay1`
- `relay2`
- `relay3`
- `relay4`
- `relay5`
- `relay6`
- `relay7`
- `relay8`

**Note:** The actual number of available relays depends on the device capabilities, which can be queried via BLE during pairing.

### Server to Client Events

#### Device Update
```javascript
socket.on('deviceUpdate', (data) => {
  // data = { topic: 'home/light', state: 'ON' }
});
```

#### Initial State
```javascript
socket.on('initialState', (data) => {
  // data = { switch: 'OFF', light: 'ON', ... }
});
```

#### Device Connection Update
```javascript
socket.on('deviceConnectionUpdate', (data) => {
  // data = { connected: true }
});
```

#### Connection Error
```javascript
socket.on('error', (data) => {
  // data = { message: 'Error description' }
});
```

## MQTT Topics

### Control Topics

All topics now use device IDs as prefixes (e.g., `st-000002/relay1`) instead of a generic prefix. This allows for multiple devices to be controlled independently.

| Topic | Description | Payload | QoS | Retained |
|-------|-------------|---------|-----|----------|
| `{deviceID}/relay1` | Relay 1 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay2` | Relay 2 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay3` | Relay 3 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay4` | Relay 4 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay5` | Relay 5 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay6` | Relay 6 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay7` | Relay 7 control | ON/OFF | 1 | Yes |
| `{deviceID}/relay8` | Relay 8 control | ON/OFF | 1 | Yes |
| `{deviceID}/temperature` | Temperature reading | Float (°C) | 0 | No |
| `{deviceID}/humidity` | Humidity reading | Float (%) | 0 | No |
| `{deviceID}/ldr` | Light sensor reading | Integer | 0 | No |
| `{deviceID}/availability` | Connection status | online/offline | 1 | Yes |

Where `{deviceID}` is the unique identifier for each device (e.g., `st-000002`).

## Code Examples

### JavaScript/Node.js

#### Control Device via Socket.IO
```javascript
const io = require('socket.io-client');
const socket = io('http://localhost:3000');

// Turn on a relay for a specific device
socket.emit('controlDevice', {
  deviceId: 'st-000002',  // Device identifier
  relay: 'relay1',        // Relay identifier
  state: 'ON'            // 'ON' or 'OFF'
});

// Listen for updates
socket.on('deviceUpdate', (data) => {
  console.log(`Device ${data.deviceId} relay ${data.relay} is now ${data.state}`);
});
```

#### MQTT Control Example
```javascript
const mqtt = require('mqtt');
const client = mqtt.connect('mqtts://your-broker:8883', {
  username: 'your-username',
  password: 'your-password'
});

// Device ID for the target device
const deviceID = 'st-000002';

// Turn on relay 1
client.publish(`${deviceID}/relay1`, 'ON', { retain: true });

// Subscribe to all topics for this device
client.subscribe(`${deviceID}/#`);
client.on('message', (topic, message) => {
  console.log(`${topic}: ${message.toString()}`);
});
```

### Python

#### REST API Example
```python
import requests

# Get device status
response = requests.get('http://localhost:3000/api/status')
status = response.json()
print(status['deviceState'])
```

#### MQTT Control Example
```python
import paho.mqtt.client as mqtt

client = mqtt.Client()
client.username_pw_set("your-username", "your-password")
client.tls_set()  # For secure connection

# Device ID for the target device
device_id = "st-000002"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribe to all topics for this device
    client.subscribe(f"{device_id}/#")

def on_message(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

client.on_connect = on_connect
client.on_message = on_message

client.connect("your-broker", 8883, 60)
client.loop_start()

# Turn on relay 1
client.publish(f"{device_id}/relay1", "ON", retain=True)
```

## Rate Limits and Throttling

- Socket.IO events: 10 requests per second per client
- MQTT messages: 2 second throttle per device
- REST API: 100 requests per minute per IP

## Error Handling

### HTTP Status Codes
- `200`: Success
- `400`: Bad Request
- `401`: Unauthorized
- `404`: Not Found
- `429`: Too Many Requests
- `500`: Internal Server Error

### Socket.IO Errors
```javascript
socket.on('error', (error) => {
  switch(error.code) {
    case 'DEVICE_OFFLINE':
      console.log('Device is not connected');
      break;
    case 'INVALID_STATE':
      console.log('Invalid device state');
      break;
    case 'THROTTLED':
      console.log('Command throttled');
      break;
    case 'INVALID_DEVICE_ID':
      console.log('Device ID not recognized');
      break;
    case 'RELAY_NOT_FOUND':
      console.log('Specified relay not found on device');
      break;
  }
});
```

## BLE Service API

The firmware now includes BLE (Bluetooth Low Energy) functionality for device setup and WiFi configuration.

### BLE Service UUID

```
4fafc201-1fb5-459e-8fcc-c5c9c331914b
```

### Characteristics

#### Device Information Characteristic (Read)

```
UUID: beb5483e-36e1-4688-b7f5-ea07361b26a8
```

Returns device information in the format: `deviceID:numRelays`

Example: `st-000002:8`

#### WiFi Configuration Characteristic (Read/Write)

```
UUID: beb5483e-36e1-4688-b7f5-ea07361b26a9
```

**Write Format**: `SSID:password`

**Read Response**: 
- Success: `Connected to WiFi. IP: 192.168.1.100`
- Failure: `Error: Could not connect to WiFi`

### BLE Connection Flow

1. Scan for devices advertising the Homato service UUID
2. Connect to the device
3. Read the device information characteristic
4. Write WiFi credentials to the WiFi configuration characteristic
5. Read the response from the WiFi configuration characteristic

### BLE API Example (JavaScript)

```javascript
// Using Web Bluetooth API
async function connectToDevice() {
  try {
    const device = await navigator.bluetooth.requestDevice({
      filters: [{
        services: ['4fafc201-1fb5-459e-8fcc-c5c9c331914b']
      }]
    });
    
    const server = await device.gatt.connect();
    const service = await server.getPrimaryService('4fafc201-1fb5-459e-8fcc-c5c9c331914b');
    
    // Read device info
    const deviceInfoChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a8');
    const infoValue = await deviceInfoChar.readValue();
    const deviceInfo = new TextDecoder().decode(infoValue);
    console.log('Device Info:', deviceInfo);
    
    // Send WiFi credentials
    const wifiConfigChar = await service.getCharacteristic('beb5483e-36e1-4688-b7f5-ea07361b26a9');
    const credentials = 'MyWiFi:MyPassword';
    await wifiConfigChar.writeValue(new TextEncoder().encode(credentials));
    
    // Read response
    const responseValue = await wifiConfigChar.readValue();
    const response = new TextDecoder().decode(responseValue);
    console.log('WiFi Config Response:', response);
    
    return deviceInfo;
  } catch (error) {
    console.error('Bluetooth Error:', error);
    throw error;
  }
}
```

## Security

### Authentication
- MQTT: Username/Password
- Socket.IO: Token-based (optional)
- REST API: API Key (optional)
- BLE: Physical access required (proximity-based security)

### SSL/TLS
- MQTT: Port 8883 with SSL/TLS
- WebSocket: WSS support
- REST API: HTTPS support
- BLE: Standard Bluetooth security

## Best Practices

1. **Error Handling**
   - Always implement error handling
   - Handle connection losses gracefully
   - Implement reconnection logic
   - Handle multi-network failover scenarios

2. **State Management**
   - Cache device states locally
   - Implement state synchronization
   - Handle offline scenarios
   - Track device IDs and capabilities

3. **Performance**
   - Use appropriate QoS levels
   - Implement message throttling
   - Monitor connection health
   - Use wildcard topics efficiently (`deviceID/#`)

4. **Security**
   - Use SSL/TLS for MQTT
   - Implement authentication
   - Validate all inputs
   - Secure BLE pairing process
   - Implement timeout for BLE pairing mode

5. **Multiple Device Management**
   - Use consistent device ID naming scheme
   - Implement device discovery mechanisms
   - Store device mappings persistently
   - Handle device additions and removals gracefully 