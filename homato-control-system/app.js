// app.js - Main server file
const express = require('express');
const http = require('http');
const mqtt = require('mqtt');
const cors = require('cors');
const socketIo = require('socket.io');
const path = require('path');
const fs = require('fs');

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
const MQTT_HOST = process.env.MQTT_HOST || 'a9de91952e404a93bc1b4be0175fd299.s1.eu.hivemq.cloud';
const MQTT_PORT = process.env.MQTT_PORT || 8883;
const MQTT_USERNAME = process.env.MQTT_USERNAME || 'rrdevices_RO_Plants';
const MQTT_PASSWORD = process.env.MQTT_PASSWORD || 'RRdevices@123';
const MQTT_CLIENT_ID = 'webapp_backend_' + Math.random().toString(16).substring(2, 8);

// Define MQTT topics
const TOPICS = {
  switch: 'home/switch',
  light: 'home/light',
  status: 'home/status',
  availability: 'home/availability' // New topic for device availability
};

// Store device states
const deviceState = {
  switch: 'OFF',
  light: 'OFF',
  deviceConnected: false // Track device connection status
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
      
      const topic = data.device === 'switch' ? TOPICS.switch : TOPICS.light;
      const state = data.state.toUpperCase();
      
      // Publish to MQTT
      mqttClient.publish(topic, state);
      
      // Update local state
      if (data.device === 'switch') {
        deviceState.switch = state;
      } else if (data.device === 'light') {
        deviceState.light = state;
      }
      
      // Broadcast to all clients
      io.emit('deviceUpdate', { topic, state });
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