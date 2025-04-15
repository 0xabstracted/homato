# API Documentation

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

**Available Devices:**
- `switch`
- `light`
- `fan`
- `tubelight`
- `bedlight`
- `falseceiling`
- `ac`
- `switchport`

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

All topics use the prefix `home/`

| Topic | Description | Payload | QoS | Retained |
|-------|-------------|---------|-----|----------|
| `home/switch` | Main switch | ON/OFF | 1 | Yes |
| `home/light` | Light control | ON/OFF | 1 | Yes |
| `home/fan` | Fan control | ON/OFF | 1 | Yes |
| `home/tubelight` | Tube light | ON/OFF | 1 | Yes |
| `home/bedlight` | Bed light | ON/OFF | 1 | Yes |
| `home/falseceiling` | False ceiling | ON/OFF | 1 | Yes |
| `home/ac` | AC control | ON/OFF | 1 | Yes |
| `home/switchport` | Switch port | ON/OFF | 1 | Yes |
| `home/status` | Device status | String | 0 | No |
| `home/availability` | Connection status | online/offline | 1 | Yes |

## Code Examples

### JavaScript/Node.js

#### Control Device via Socket.IO
```javascript
const io = require('socket.io-client');
const socket = io('http://localhost:3000');

// Turn on the light
socket.emit('controlDevice', {
  device: 'light',
  state: 'ON'
});

// Listen for updates
socket.on('deviceUpdate', (data) => {
  console.log(`Device ${data.topic} is now ${data.state}`);
});
```

#### MQTT Control Example
```javascript
const mqtt = require('mqtt');
const client = mqtt.connect('mqtts://your-broker:8883', {
  username: 'your-username',
  password: 'your-password'
});

// Turn on the light
client.publish('home/light', 'ON', { retain: true });

// Subscribe to updates
client.subscribe('home/#');
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

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("home/#")

def on_message(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

client.on_connect = on_connect
client.on_message = on_message

client.connect("your-broker", 8883, 60)
client.loop_start()

# Turn on the light
client.publish("home/light", "ON", retain=True)
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
  }
});
```

## Security

### Authentication
- MQTT: Username/Password
- Socket.IO: Token-based (optional)
- REST API: API Key (optional)

### SSL/TLS
- MQTT: Port 8883 with SSL/TLS
- WebSocket: WSS support
- REST API: HTTPS support

## Best Practices

1. **Error Handling**
   - Always implement error handling
   - Handle connection losses gracefully
   - Implement reconnection logic

2. **State Management**
   - Cache device states locally
   - Implement state synchronization
   - Handle offline scenarios

3. **Performance**
   - Use appropriate QoS levels
   - Implement message throttling
   - Monitor connection health

4. **Security**
   - Use SSL/TLS
   - Implement authentication
   - Validate all inputs 