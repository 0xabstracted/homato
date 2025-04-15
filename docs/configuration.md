# Configuration Guide

## Backend Configuration

### Environment Variables

Create a `.env` file in the root directory with the following variables:

```env
PORT=3000
MQTT_HOST=your-mqtt-host
MQTT_PORT=8883
MQTT_USERNAME=your-username
MQTT_PASSWORD=your-password
DEBUG=app:*
```

### MQTT Topics

| Topic | Description | Values | QoS | Retained |
|-------|-------------|--------|-----|----------|
| home/switch | Main switch control | ON/OFF | 1 | Yes |
| home/light | Light control | ON/OFF | 1 | Yes |
| home/fan | Fan control | ON/OFF | 1 | Yes |
| home/tubelight | Tube light control | ON/OFF | 1 | Yes |
| home/bedlight | Bed light control | ON/OFF | 1 | Yes |
| home/falseceiling | False ceiling control | ON/OFF | 1 | Yes |
| home/ac | AC control | ON/OFF | 1 | Yes |
| home/switchport | Switch port control | ON/OFF | 1 | Yes |
| home/status | Device status | String | 0 | No |
| home/availability | Connection status | online/offline | 1 | Yes |

## Firmware Configuration

### WiFi Settings
```cpp
// In homato_v1.ino
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
```

### MQTT Settings
```cpp
const char* mqtt_server = "your-mqtt-host";
const int mqtt_port = 8883;
const char* mqtt_username = "your-username";
const char* mqtt_password = "your-password";
```

### GPIO Configuration
```cpp
// Relay pin assignments
const int switchPin = D1;    // GPIO5
const int lightPin = D2;     // GPIO4
const int fanPin = D3;       // GPIO0
const int tubelightPin = D4; // GPIO2
// ... add more pins as needed
```

## Web Interface Configuration

### Socket.IO Settings
```javascript
// In public/app.js
const socket = io({
  reconnectionDelay: 1000,
  reconnectionDelayMax: 5000,
  reconnectionAttempts: Infinity
});
```

### UI Customization
```css
/* In public/styles.css */
:root {
  --primary-color: #007bff;
  --secondary-color: #6c757d;
  --success-color: #28a745;
  --danger-color: #dc3545;
  --background-color: #f8f9fa;
}
```

## Security Configuration

### SSL/TLS Settings
```javascript
// In app.js
const options = {
  key: fs.readFileSync('path/to/private.key'),
  cert: fs.readFileSync('path/to/certificate.crt'),
  ca: fs.readFileSync('path/to/ca.crt')
};
```

### MQTT Security
```cpp
// In homato_v1.ino
// For production, replace setInsecure() with proper certificate validation
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"... Your CA Certificate ...\n" \
"-----END CERTIFICATE-----\n";

void setup() {
  // ...
  espClient.setCACert(root_ca);
  // ...
}
```

## Docker Configuration

### Docker Build
```dockerfile
# In Dockerfile
FROM node:20
WORKDIR /app
COPY homato-control-system/ .
RUN npm install
EXPOSE 3000
CMD ["node", "app.js"]
```

### Docker Run
```bash
# In run.sh
sudo docker build -t app .
sudo docker run -d -it -p 3000:3000 --name appc app
```

## Development Tools

### Nodemon Configuration
```json
{
  "watch": ["app.js", "routes/", "public/"],
  "ext": "js,json,html,css",
  "ignore": ["node_modules/"],
  "exec": "node app.js"
}
```

### Debug Configuration
```javascript
// Enable debug logs
process.env.DEBUG = 'app:*,socket:*,mqtt:*';
```

## Scaling Configuration

### PM2 Process Manager
```json
// ecosystem.config.js
module.exports = {
  apps: [{
    name: "homato",
    script: "app.js",
    instances: "max",
    exec_mode: "cluster",
    env: {
      NODE_ENV: "production",
      PORT: 3000
    }
  }]
}
``` 