// app.js - Main server file
const express = require('express');
const http = require('http');
const mqtt = require('mqtt');
const cors = require('cors');
const socketIo = require('socket.io');
const path = require('path');
const fs = require('fs');
const dotenv = require('dotenv');

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

// Environment variables (set these with actual values or use a .env file with dotenv)
const PORT = process.env.PORT || 3000;
const MQTT_HOST = process.env.MQTT_HOST;
const MQTT_PORT = process.env.MQTT_PORT;
const MQTT_USERNAME = process.env.MQTT_USERNAME;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;
const MQTT_CLIENT_ID = 'webapp_backend_' + Math.random().toString(16).substring(2, 8);

// console.log(MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_CLIENT_ID);

// Define MQTT topics
const TOPICS = {
  switch: 'home/switch',
  light: 'home/light',
  fan: 'home/fan',
  tubelight: 'home/tubelight',
  bedlight: 'home/bedlight',
  falseceiling: 'home/falseceiling',
  ac: 'home/ac',
  switchport: 'home/switchport',
  status: 'home/status',
  availability: 'home/availability' // New topic for device availability
};

// Store device states
const deviceState = {
  switch: 'OFF',
  light: 'OFF',
  fan: 'OFF',
  tubelight: 'OFF',
  bedlight: 'OFF',
  falseceiling: 'OFF',
  ac: 'OFF',
  switchport: 'OFF',
  deviceConnected: false // Track device connection status
};

// Throttling variables
const throttleTime = 2000; // 2 seconds
const pendingCommands = {
  switch: null,
  light: null,
  fan: null,
  tubelight: null,
  bedlight: null,
  falseceiling: null,
  ac: null,
  switchport: null
};
const deviceTimers = {
  switch: null,
  light: null,
  fan: null,
  tubelight: null,
  bedlight: null,
  falseceiling: null,
  ac: null,
  switchport: null
};

// Connect to MQTT broker
const mqttClient = mqtt.connect(`mqtts://${MQTT_HOST}:${MQTT_PORT}`, {
  clientId: MQTT_CLIENT_ID,
  username: MQTT_USERNAME,
  password: MQTT_PASSWORD,
  rejectUnauthorized: false, // For development - remove in production or provide proper CA certificate
  keepalive: 60, // Check connection every 60 seconds
  connectTimeout: 30 * 1000 // 30 seconds timeout
});

// Handle MQTT connection
mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');

  // Subscribe to all relevant topics
  mqttClient.subscribe(TOPICS.switch);
  mqttClient.subscribe(TOPICS.light);
  mqttClient.subscribe(TOPICS.fan);
  mqttClient.subscribe(TOPICS.tubelight);
  mqttClient.subscribe(TOPICS.bedlight);
  mqttClient.subscribe(TOPICS.falseceiling);
  mqttClient.subscribe(TOPICS.ac);
  mqttClient.subscribe(TOPICS.switchport);
  mqttClient.subscribe(TOPICS.status);
  mqttClient.subscribe(TOPICS.availability); // Subscribe to availability topic

  // Publish initial connection message
  mqttClient.publish(TOPICS.status, 'Backend server online');
});

// Handle MQTT messages
mqttClient.on('message', (topic, message) => {
  const messageStr = message.toString();
  console.log(`Received message on ${topic}: ${messageStr}`);

  // Update device state based on topic
  if (topic === TOPICS.switch) {
    deviceState.switch = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.light) {
    deviceState.light = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.fan) {
    deviceState.fan = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.tubelight) {
    deviceState.tubelight = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.bedlight) {
    deviceState.bedlight = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.falseceiling) {
    deviceState.falseceiling = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.ac) {
    deviceState.ac = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.switchport) {
    deviceState.switchport = messageStr;
    io.emit('deviceUpdate', { topic, state: messageStr });
  } else if (topic === TOPICS.availability) {
    // Handle device availability updates
    const isConnected = messageStr === 'online';
    if (deviceState.deviceConnected !== isConnected) {
      deviceState.deviceConnected = isConnected;
      io.emit('deviceConnectionUpdate', { connected: isConnected });
      console.log(`Device connection status changed to: ${isConnected ? 'connected' : 'disconnected'}`);
    }
  }
});

// Function to execute throttled command
function executeCommand(device) {
  const state = pendingCommands[device];
  if (state !== null) {
    const topic = TOPICS[device];

    // Publish to MQTT
    mqttClient.publish(topic, state);

    // Update local state
    deviceState[device] = state;

    // Broadcast to all clients
    io.emit('deviceUpdate', { topic, state });

    // Clear the pending command
    pendingCommands[device] = null;
    console.log(`Executed throttled command: ${device} -> ${state}`);
  }

  // Clear the timer
  deviceTimers[device] = null;
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
  // When MQTT connection is lost, mark device as offline
  if (deviceState.deviceConnected) {
    deviceState.deviceConnected = false;
    io.emit('deviceConnectionUpdate', { connected: false });
    console.log('Device marked as disconnected due to MQTT disconnection');
  }
});

// Socket.io connection
io.on('connection', (socket) => {
  console.log('New client connected');

  // Send current state to newly connected client
  socket.emit('initialState', deviceState);

  // Handle control events from frontend
  socket.on('controlDevice', (data) => {
    console.log('Control request received:', data);

    if (data && data.device && data.state) {
      // Only process control requests if device is connected
      if (!deviceState.deviceConnected) {
        socket.emit('error', { message: 'Device is offline, cannot process command' });
        return;
      }

      const device = data.device;
      const state = data.state.toUpperCase();

      // If current state is already what we want, do nothing
      if (deviceState[device] === state) {
        console.log(`Device ${device} already in state ${state}, ignoring`);
        return;
      }

      // Store the latest command in the pending queue
      pendingCommands[device] = state;

      // If there's no active timer for this device, set one up
      if (!deviceTimers[device]) {
        console.log(`Setting up timer for ${device}`);
        deviceTimers[device] = setTimeout(() => executeCommand(device), throttleTime);
      } else {
        console.log(`Command for ${device} queued, will execute after current throttle window`);
      }
    }
  });

  // Handle disconnection
  socket.on('disconnect', () => {
    console.log('Client disconnected');
  });
});

// API endpoints
app.get('/api/status', (req, res) => {
  res.json({ deviceState });
});

// Serve the main HTML file
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start the server
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});