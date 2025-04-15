// app.js - Main server file
const express = require('express');
const http = require('http');
const mqtt = require('mqtt');
const cors = require('cors');
const socketIo = require('socket.io');
const path = require('path');
const fs = require('fs');
const mysql = require('mysql2/promise');
const bcrypt = require('bcrypt');
const jwt = require('jsonwebtoken');
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

// MySQL Connection Pool
const pool = mysql.createPool({
  host: process.env.MYSQL_HOST || 'localhost',
  user: process.env.MYSQL_USER || 'homato_user',
  password: process.env.MYSQL_PASSWORD || 'homato_password',
  database: process.env.MYSQL_DATABASE || 'homato_db',
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
});

// Middleware
app.use(cors());
app.use(express.json());
// app.use(express.static(path.join(__dirname, 'public')));

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

// Authentication middleware
const authenticateToken = async (req, res, next) => {
  const authHeader = req.headers['authorization'];
  const token = authHeader && authHeader.split(' ')[1];

  if (!token) {
    return res.status(401).json({ message: 'No token provided' });
  }

  try {
    const decoded = jwt.verify(token, process.env.JWT_SECRET);
    const [rows] = await pool.execute(
      'SELECT * FROM sessions WHERE token = ? AND expires_at > NOW()',
      [token]
    );

    if (rows.length === 0) {
      return res.status(401).json({ message: 'Invalid or expired token' });
    }

    req.user = decoded;
    next();
  } catch (error) {
    return res.status(403).json({ message: 'Invalid token' });
  }
};

// Login endpoint
app.post('/api/auth/login', async (req, res) => {
  const { username, password } = req.body;

  try {
    const [rows] = await pool.execute(
      'SELECT * FROM users WHERE username = ?',
      [username]
    );

    if (rows.length === 0) {
      return res.status(401).json({ message: 'Invalid credentials' });
    }

    const user = rows[0];
    const validPassword = await bcrypt.compare(password, user.password);

    if (!validPassword) {
      return res.status(401).json({ message: 'Invalid credentials' });
    }

    const token = jwt.sign({ id: user.id, username: user.username }, process.env.JWT_SECRET, { expiresIn: '24h' });

    // Store session
    await pool.execute(
      'INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, DATE_ADD(NOW(), INTERVAL 24 HOUR))',
      [user.id, token]
    );

    res.json({ token });
  } catch (error) {
    console.error('Login error:', error);
    res.status(500).json({ message: 'Internal server error' });
  }
});

// Logout endpoint
app.post('/api/auth/logout', authenticateToken, async (req, res) => {
  try {
    await pool.execute(
      'DELETE FROM sessions WHERE user_id = ?',
      [req.user.id]
    );
    res.json({ message: 'Logged out successfully' });
  } catch (error) {
    console.error('Logout error:', error);
    res.status(500).json({ message: 'Internal server error' });
  }
});

// Serve login page
app.get('/login', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'login.html'));
});

// Redirect root to login if not authenticated
app.get('/', (req, res) => {
  const token = req.headers.authorization?.split(' ')[1];
  if (!token) {
    res.redirect('/login');
  } else {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
  }
});

// Protected API endpoints
app.get('/api/status', authenticateToken, (req, res) => {
  res.json({ deviceState });
});

// Start the server
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});