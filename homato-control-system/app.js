// app.js - Main server file
const express = require('express');
const http = require('http');
const mqtt = require('mqtt');
const cors = require('cors');
const socketIo = require('socket.io');
const path = require('path');
const fs = require('fs');
const dotenv = require('dotenv');
const connectDB = require('./src/config/database');
const authRoutes = require('./src/routes/auth');
const deviceRoutes = require('./src/routes/device');

// Load environment variables from .env file
dotenv.config();

// Create Express app
const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// Routes
app.use('/api/auth', authRoutes);
app.use('/api/devices', deviceRoutes);

// Environment variables (set these with actual values or use a .env file with dotenv)
const PORT = process.env.PORT || 3000;
const MQTT_HOST = process.env.MQTT_HOST;
const MQTT_PORT = process.env.MQTT_PORT;
const MQTT_USERNAME = process.env.MQTT_USERNAME;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;
const MQTT_CLIENT_ID = 'webapp_backend_' + Math.random().toString(16).substring(2, 8);

// Number of relays per device
const RELAYS_PER_DEVICE = 18;

// Device registry - stores all connected devices and their states
const deviceRegistry = {};

// Track connected frontend clients
const connectedClients = new Set();

// Throttling mechanism - stores pending commands and timers for each device/relay
const pendingCommands = {};
const deviceTimers = {};
const throttleTime = 500; // 500ms throttle time

// Connect to MQTT broker
const mqttClient = mqtt.connect(`mqtts://${MQTT_HOST}:${MQTT_PORT}`, {
  clientId: MQTT_CLIENT_ID,
  username: MQTT_USERNAME,
  password: MQTT_PASSWORD,
  rejectUnauthorized: false, // For development - remove in production or provide proper CA certificate
  keepalive: 60, // Check connection every 60 seconds
  connectTimeout: 30 * 1000 // 30 seconds timeout
});

// Connect to MongoDB
connectDB();

// Handle MQTT connection
mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  console.log(`MQTT Client ID: ${MQTT_CLIENT_ID}`);
  console.log(`MQTT Host: ${MQTT_HOST}:${MQTT_PORT}`);

  // Subscribe to each relay topic individually
  for (let i = 1; i <= RELAYS_PER_DEVICE; i++) {
    mqttClient.subscribe(`+/relay${i}`, { qos: 2 }, (err, granted) => {
      if (err) {
        console.error(`Error subscribing to relay${i} topics:`, err);
      } else {
        console.log(`Successfully subscribed to relay${i} topics:`, granted);
      }
    });
  }

  mqttClient.subscribe('+/availability', { qos: 2 }, (err, granted) => {
    if (err) {
      console.error('Error subscribing to availability topics:', err);
    } else {
      console.log('Successfully subscribed to availability topics:', granted);
    }
  });

  // Publish initial connection message
  mqttClient.publish('server/status', 'Backend server online');

});

// Debug subscription state
mqttClient.on('subscribe', function (topic, granted) {
  console.log(`Subscription confirmed for: ${JSON.stringify(topic)}, QoS: ${JSON.stringify(granted)}`);
});

// Process a topic to extract deviceId and relay number
function processTopic(topic) {
  const parts = topic.split('/');
  if (parts.length !== 2) return null;

  const deviceId = parts[0];
  const endpoint = parts[1];

  return { deviceId, endpoint };
}

// Initialize device structure if it doesn't exist
function initializeDevice(deviceId) {
  if (!deviceRegistry[deviceId]) {
    deviceRegistry[deviceId] = {
      relays: {},
      online: false,
      status: 'offline', // initial status
      lastSeen: Date.now()
    };

    // Initialize all relays to OFF
    for (let i = 1; i <= RELAYS_PER_DEVICE; i++) {
      deviceRegistry[deviceId].relays[`relay${i}`] = 'OFF';
    }

    // Initialize throttling structures for this device
    pendingCommands[deviceId] = {};
    deviceTimers[deviceId] = {};

    for (let i = 1; i <= RELAYS_PER_DEVICE; i++) {
      pendingCommands[deviceId][`relay${i}`] = null;
      deviceTimers[deviceId][`relay${i}`] = null;
    }

    console.log(`New device registered: ${deviceId}`);
  }
  return deviceRegistry[deviceId];
}

// Send device state to all connected clients
function broadcastDeviceState(deviceId) {
  if (deviceRegistry[deviceId] && connectedClients.size > 0) {
    const deviceState = {
      deviceId: deviceId,
      online: deviceRegistry[deviceId].online,
      status: deviceRegistry[deviceId].status,
      relays: deviceRegistry[deviceId].relays
    };

    io.emit('deviceState', deviceState);
    console.log(`Broadcasted state for device ${deviceId} to ${connectedClients.size} clients`);
  }
}

// Handle MQTT messages
mqttClient.on('message', (topic, message) => {
  const messageStr = message.toString();
  const now = new Date();
  const istTime = new Date(now.getTime() + (5.5 * 60 * 60 * 1000));
  console.log(`Received message on ${topic}: ${messageStr} at ${istTime.toISOString().replace('T', ' ').substring(0, 19)}`);

  // Log raw topic parts for debugging
  console.log(`Topic parts: ${JSON.stringify(topic.split('/'))}`);

  const topicInfo = processTopic(topic);
  if (!topicInfo) {
    console.log(`Invalid topic format: ${topic}`);
    return;
  }

  const { deviceId, endpoint } = topicInfo;
  console.log(`Processed topic - deviceId: ${deviceId}, endpoint: ${endpoint}`);

  // Initialize device if it doesn't exist in registry
  const device = initializeDevice(deviceId);

  // Update device's lastSeen timestamp
  device.lastSeen = Date.now();

  if (endpoint === 'availability') {
    // Handle device availability updates
    const isOnline = messageStr === 'online';
    if (device.online !== isOnline) {
      device.online = isOnline;
      device.status = isOnline ? 'online' : 'offline';
      io.emit('deviceConnectionUpdate', {
        deviceId,
        connected: isOnline,
        status: device.status
      });
      const istTime = new Date(now.getTime() + (5.5 * 60 * 60 * 1000));
      console.log(`Device ${deviceId} connection status changed to: ${isOnline ? 'connected' : 'disconnected'} at ${istTime.toISOString().replace('T', ' ').substring(0, 19)}`);

      // If device comes online, broadcast its full state
      if (isOnline) {
        broadcastDeviceState(deviceId);
      }
    }
  } else if (endpoint.startsWith('relay')) {
    // Handle relay state updates
    console.log(`Updating relay state: ${deviceId}/${endpoint} -> ${messageStr}`);
    device.relays[endpoint] = messageStr;
    io.emit('deviceUpdate', {
      deviceId,
      relay: endpoint,
      state: messageStr
    });

    // If this is a new relay state, also broadcast the full device state
    broadcastDeviceState(deviceId);
  } else {
    console.log(`Unhandled endpoint type: ${endpoint} for device ${deviceId}`);
  }
});

// Function to execute throttled command
function executeCommand(deviceId, relay) {
  const state = pendingCommands[deviceId][relay];
  if (state !== null) {
    const topic = `${deviceId}/${relay}`;

    // Publish to MQTT with QoS 1 to ensure delivery
    mqttClient.publish(topic, state, { qos: 2 });

    // Update local state
    if (deviceRegistry[deviceId]) {
      deviceRegistry[deviceId].relays[relay] = state;
    }

    // Broadcast to all clients
    io.emit('deviceUpdate', {
      deviceId,
      relay,
      state
    });

    // Clear the pending command
    pendingCommands[deviceId][relay] = null;
    console.log(`Executed throttled command: ${deviceId}/${relay} -> ${state}`);
  }

  // Clear the timer
  deviceTimers[deviceId][relay] = null;
}

// Handle MQTT reconnection
mqttClient.on('reconnect', () => {
  console.log('Attempting to reconnect to MQTT broker...');
});

// Handle MQTT errors
mqttClient.on('error', (err) => {
  console.error('MQTT connection error:', err);
});

// Handle MQTT disconnection
mqttClient.on('close', () => {
  console.log('Disconnected from MQTT broker');

  // Mark all devices as offline when MQTT connection is lost
  Object.keys(deviceRegistry).forEach(deviceId => {
    if (deviceRegistry[deviceId].online) {
      deviceRegistry[deviceId].online = false;
      deviceRegistry[deviceId].status = 'offline';
      io.emit('deviceConnectionUpdate', {
        deviceId,
        connected: false,
        status: 'offline'
      });
    }
  });

  console.log('All devices marked as disconnected due to MQTT disconnection');
});

// Socket.io connection
io.on('connection', (socket) => {
  console.log('New Socket.io client connected');

  // Add client to connected clients set
  connectedClients.add(socket.id);
  console.log(`Client ${socket.id} connected. Total clients: ${connectedClients.size}`);

  // Send current state of all devices to newly connected client
  socket.emit('initialState', { devices: deviceRegistry });

  // Send individual device states for each device
  Object.keys(deviceRegistry).forEach(deviceId => {
    if (deviceRegistry[deviceId].online) {
      socket.emit('deviceState', {
        deviceId: deviceId,
        online: deviceRegistry[deviceId].online,
        status: deviceRegistry[deviceId].status,
        relays: deviceRegistry[deviceId].relays
      });
    }
  });

  // Handle getDevices request (for reconnection)
  socket.on('getDevices', () => {
    socket.emit('initialState', { devices: deviceRegistry });
    console.log('Client requested device list refresh');
  });

  // Handle specific device state request
  socket.on('getDeviceState', (deviceId) => {
    if (deviceRegistry[deviceId]) {
      socket.emit('deviceState', {
        deviceId: deviceId,
        online: deviceRegistry[deviceId].online,
        status: deviceRegistry[deviceId].status,
        relays: deviceRegistry[deviceId].relays
      });
      console.log(`Client requested state for device ${deviceId}`);
    } else {
      socket.emit('error', { message: `Device ${deviceId} not found` });
    }
  });

  // Handle control events from frontend
  socket.on('controlDevice', (data) => {
    const now = new Date();
    const istTime = new Date(now.getTime() + (5.5 * 60 * 60 * 1000));
    console.log('Control request received:', data, 'at', istTime.toISOString().replace('T', ' ').substring(0, 19));

    if (data && data.deviceId && data.relay && data.state) {
      const { deviceId, relay, state } = data;
      const stateUpper = state.toUpperCase();

      // Check if device exists
      if (!deviceRegistry[deviceId]) {
        socket.emit('error', { message: `Device ${deviceId} is not registered` });
        return;
      }

      // Check if device is online
      if (!deviceRegistry[deviceId].online) {
        socket.emit('error', { message: `Device ${deviceId} is offline, cannot process command` });
        return;
      }

      // Check if relay exists
      if (!deviceRegistry[deviceId].relays.hasOwnProperty(relay)) {
        socket.emit('error', { message: `Relay ${relay} does not exist on device ${deviceId}` });
        return;
      }

      // If current state is already what we want, do nothing
      if (deviceRegistry[deviceId].relays[relay] === stateUpper) {
        console.log(`Relay ${relay} on device ${deviceId} already in state ${stateUpper}, ignoring`);
        return;
      }

      // Create throttling structures if they don't exist
      if (!pendingCommands[deviceId]) {
        pendingCommands[deviceId] = {};
      }
      if (!deviceTimers[deviceId]) {
        deviceTimers[deviceId] = {};
      }
      if (!pendingCommands[deviceId][relay]) {
        pendingCommands[deviceId][relay] = null;
      }
      if (!deviceTimers[deviceId][relay]) {
        deviceTimers[deviceId][relay] = null;
      }

      // Store the latest command in the pending queue
      pendingCommands[deviceId][relay] = stateUpper;

      // If there's no active timer for this relay, set one up
      if (!deviceTimers[deviceId][relay]) {
        console.log(`Setting up timer for ${deviceId}/${relay}`);
        deviceTimers[deviceId][relay] = executeCommand(deviceId, relay);
        // deviceTimers[deviceId][relay] = setTimeout(() => executeCommand(deviceId, relay), throttleTime);
        console.log(`Timer set for ${deviceId}/${relay} to execute in ${throttleTime}ms`);
      } else {
        console.log(`Command for ${deviceId}/${relay} queued, will execute after current throttle window`);
      }
    } else {
      socket.emit('error', { message: 'Invalid control request' });
    }
  });

  // Handle disconnection
  socket.on('disconnect', () => {
    // Remove client from connected clients set
    connectedClients.delete(socket.id);
    console.log(`Client ${socket.id} disconnected. Remaining clients: ${connectedClients.size}`);
  });
});

// API endpoints
app.get('/api/devices', (req, res) => {
  res.json({ devices: deviceRegistry });
});

app.get('/api/devices/:deviceId', (req, res) => {
  const { deviceId } = req.params;
  if (deviceRegistry[deviceId]) {
    res.json({ device: deviceRegistry[deviceId] });
  } else {
    res.status(404).json({ error: 'Device not found' });
  }
});

// Serve the main HTML file
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Device status check mechanism - mark devices as unreachable after inactivity
const DEVICE_UNREACHABLE_TIMEOUT = 3600000; // 1 hour
const DEVICE_CLEANUP_TIMEOUT = 7 * 24 * 3600000; // 7 days (for potential actual cleanup in the future)

setInterval(() => {
  const now = Date.now();
  Object.keys(deviceRegistry).forEach(deviceId => {
    const device = deviceRegistry[deviceId];
    const inactiveTime = now - device.lastSeen;

    if (inactiveTime > DEVICE_UNREACHABLE_TIMEOUT) {
      // If device was online, update status and notify clients
      if (device.online || device.status !== 'unreachable') {
        console.log(`Marking device as unreachable: ${deviceId}`);
        device.online = false;
        device.status = 'unreachable';

        // Notify all connected clients about status change
        io.emit('deviceConnectionUpdate', {
          deviceId,
          connected: false,
          status: 'unreachable'
        });
      }

      // We don't delete the device, just mark it unreachable
      // This preserves device history and allows reconnection
    }
  });
}, 300000); // Check every 5 minutes

// Start the server
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});