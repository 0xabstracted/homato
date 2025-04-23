import React, { createContext, useContext, useEffect, useState, ReactNode } from 'react';
import { io, Socket } from 'socket.io-client';

// Define the structure of our device state
export interface DeviceState {
  switch: string;
  light: string;
  fan: string;
  tubelight: string;
  bedlight: string;
  falseceiling: string;
  ac: string;
  switchport: string;
  deviceConnected: boolean;
}

// Define device type for control functions
export type DeviceType = 'switch' | 'light' | 'fan' | 'tubelight' | 'bedlight' | 'falseceiling' | 'ac' | 'switchport';

// Define activity log item type
export interface ActivityLogItem {
  time: string;
  message: string;
}

// Define the context interface
interface HomeAutomationContextType {
  deviceState: DeviceState;
  isConnected: boolean;
  controlDevice: (device: DeviceType, state: 'ON' | 'OFF') => void;
  isLoading: boolean;
  error: string | null;
  refreshConnection: () => void;
  activityLog: ActivityLogItem[];
}

// Default context state
const defaultContextValue: HomeAutomationContextType = {
  deviceState: {
    switch: 'OFF',
    light: 'OFF',
    fan: 'OFF',
    tubelight: 'OFF',
    bedlight: 'OFF',
    falseceiling: 'OFF',
    ac: 'OFF',
    switchport: 'OFF',
    deviceConnected: false,
  },
  isConnected: false,
  controlDevice: () => {},
  isLoading: true,
  error: null,
  refreshConnection: () => {},
  activityLog: [],
};

// Create context
const HomeAutomationContext = createContext<HomeAutomationContextType>(defaultContextValue);

// Server URL - replace with your actual server URL
const SERVER_URL = 'http://54.226.69.130:3000';

// Provider component
export function HomeAutomationProvider({ children }: { children: ReactNode }) {
  const [socket, setSocket] = useState<Socket | null>(null);
  const [deviceState, setDeviceState] = useState<DeviceState>(defaultContextValue.deviceState);
  const [isConnected, setIsConnected] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [activityLog, setActivityLog] = useState<ActivityLogItem[]>([]);

  const addToActivityLog = (message: string) => {
    const timestamp = new Date().toLocaleTimeString();
    setActivityLog(prev => [{ time: timestamp, message }, ...prev].slice(0, 50));
  };

  // Initialize socket connection
  const connectSocket = () => {
    setIsLoading(true);
    
    // Create socket connection
    const newSocket = io(SERVER_URL, {
      transports: ["websocket"],
      reconnection: true,
      reconnectionAttempts: 5,
      reconnectionDelay: 1000,
    });
    
    setSocket(newSocket);

    // Set up event handlers
    newSocket.on('connect', () => {
      console.log('Connected to socket server');
      setIsConnected(true);
      setError(null);
      addToActivityLog('Connected to server');
    });

    newSocket.on('disconnect', () => {
      console.log('Disconnected from socket server');
      setIsConnected(false);
      addToActivityLog('Disconnected from server');
    });

    newSocket.on('connect_error', (err) => {
      console.error('Socket connection error:', err);
      setError(`Connection error: ${err.message}`);
      setIsLoading(false);
      addToActivityLog(`Connection error: ${err.message}`);
    });

    // Get initial state
    newSocket.on('initialState', (state: DeviceState) => {
      console.log('Received initial state:', state);
      setDeviceState(state);
      setIsLoading(false);
      addToActivityLog('Received device status');
    });

    // Handle device updates
    newSocket.on('deviceUpdate', ({ topic, state }: { topic: string; state: string }) => {
      const deviceId = topic.split('/').pop() as keyof DeviceState;
      console.log(`Device update: ${deviceId} -> ${state}`);
      
      setDeviceState(prevState => ({
        ...prevState,
        [deviceId]: state
      }));
      
      addToActivityLog(`${deviceId} turned ${state}`);
    });

    // Handle device connection updates
    newSocket.on('deviceConnectionUpdate', ({ connected }: { connected: boolean }) => {
      console.log(`Device connection status: ${connected ? "connected" : "disconnected"}`);
      
      setDeviceState(prevState => ({
        ...prevState,
        deviceConnected: connected
      }));
      
      addToActivityLog(`Device ${connected ? "connected" : "disconnected"}`);
    });

    newSocket.on('error', (data: { message: string }) => {
      console.error("Server error:", data.message);
      setError(data.message);
      addToActivityLog(`Error: ${data.message}`);
    });
  };

  useEffect(() => {
    connectSocket();
    
    // Cleanup on unmount
    return () => {
      if (socket) {
        socket.disconnect();
      }
    };
  }, []);

  // Function to refresh connection
  const refreshConnection = () => {
    if (socket) {
      socket.disconnect();
    }
    connectSocket();
  };

  // Function to control a device
  const controlDevice = (device: DeviceType, state: 'ON' | 'OFF') => {
    if (socket && socket.connected) {
      // Optimistically update UI
      setDeviceState(prevState => ({
        ...prevState,
        [device]: state
      }));
      
      socket.emit('controlDevice', { device, state });
      addToActivityLog(`Requesting ${device} to turn ${state}`);
    } else {
      setError('Cannot control device: Socket disconnected');
      addToActivityLog('Failed to control device: disconnected');
    }
  };

  // The context value
  const contextValue: HomeAutomationContextType = {
    deviceState,
    isConnected,
    controlDevice,
    isLoading,
    error,
    refreshConnection,
    activityLog,
  };

  return (
    <HomeAutomationContext.Provider value={contextValue}>
      {children}
    </HomeAutomationContext.Provider>
  );
}

// Custom hook to use the home automation context
export function useHomeAutomation() {
  const context = useContext(HomeAutomationContext);
  if (context === undefined) {
    throw new Error('useHomeAutomation must be used within a HomeAutomationProvider');
  }
  return context;
} 