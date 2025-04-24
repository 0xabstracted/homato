// Server configuration for the Homato app
// This file contains the configuration needed to connect to the backend server

// Default server URL (same as in the backend)
export const DEFAULT_SERVER_URL = 'http://54.226.69.130:3000';

// Socket.io connection options
export const SOCKET_OPTIONS = {
  reconnectionAttempts: 5,
  reconnectionDelay: 1000,
  timeout: 10000,
  autoConnect: true,
  transports: ['websocket'],
};

// MQTT configuration (if needed directly in the app)
export const MQTT_CONFIG = {
  // These would typically be stored securely or retrieved from the server
  // rather than hardcoded in the app
  HOST: 'mqtt://broker.example.com',
  PORT: 1883,
  CLIENT_ID_PREFIX: 'mobile_app_',
};

// Number of relays per device (matching backend configuration)
export const RELAYS_PER_DEVICE = 8;

// API endpoints
export const API = {
  DEVICES: '/api/devices',
  DEVICE: (deviceId: string) => `/api/devices/${deviceId}`,
};

// Socket.io events (matching backend events)
export const SOCKET_EVENTS = {
  // Connection events
  CONNECT: 'connect',
  DISCONNECT: 'disconnect',
  ERROR: 'error',
  
  // Device data events
  INITIAL_STATE: 'initialState',
  DEVICE_UPDATE: 'deviceUpdate',
  DEVICE_CONNECTION_UPDATE: 'deviceConnectionUpdate',
  
  // Commands
  GET_DEVICES: 'getDevices',
  GET_DEVICE: 'getDevice',
  CONTROL_DEVICE: 'controlDevice',
};
