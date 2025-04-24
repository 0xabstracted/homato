import React, { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import socketService from '../services/socketService';
import deviceService, { DeviceRegistry } from '../services/deviceService';

// Context type definition
interface AppContextType {
  isConnected: boolean;
  serverUrl: string;
  devices: DeviceRegistry;
  setServerUrl: (url: string) => Promise<void>;
  connect: () => Promise<void>;
  disconnect: () => void;
  refreshDevices: () => void;
  controlDevice: (deviceId: string, relay: string, state: 'ON' | 'OFF') => void;
}

// Create context with default values
const AppContext = createContext<AppContextType>({
  isConnected: false,
  serverUrl: '',
  devices: {},
  setServerUrl: async () => {},
  connect: async () => {},
  disconnect: () => {},
  refreshDevices: () => {},
  controlDevice: () => {},
});

// Provider component
export const AppProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [isConnected, setIsConnected] = useState<boolean>(socketService.isConnected);
  const [serverUrl, setServerUrlState] = useState<string>('');
  const [devices, setDevices] = useState<DeviceRegistry>({});

  // Initialize on mount
  useEffect(() => {
    // Load server URL from AsyncStorage
    const loadServerUrl = async () => {
      try {
        // This is just to update the UI state, the service already loads it internally
        const savedUrl = await AsyncStorage.getItem('homato_server_url');
        if (savedUrl) {
          setServerUrlState(savedUrl);
        } else {
          // Use default from config if none saved
          setServerUrlState(socketService.getServerUrl());
        }
      } catch (error) {
        console.error('Failed to load server URL in context:', error);
      }
    };

    loadServerUrl();

    // Set up listeners
    socketService.addConnectionListener(setIsConnected);
    deviceService.addDeviceListener(setDevices);

    // Connect to server
    socketService.connect().catch(error => {
      console.error('Failed to connect to server:', error);
    });

    // Clean up listeners on unmount
    return () => {
      socketService.removeConnectionListener(setIsConnected);
      deviceService.removeDeviceListener(setDevices);
    };
  }, []);

  // Set server URL and reconnect
  const setServerUrl = async (url: string): Promise<void> => {
    try {
      await socketService.saveServerUrl(url);
      setServerUrlState(url);
      // Reconnect with new URL
      await socketService.connect();
    } catch (error) {
      console.error('Failed to set server URL:', error);
      throw error;
    }
  };

  // Connect to server
  const connect = async (): Promise<void> => {
    try {
      await socketService.connect();
    } catch (error) {
      console.error('Failed to connect to server:', error);
      throw error;
    }
  };

  // Disconnect from server
  const disconnect = (): void => {
    socketService.disconnect();
  };

  // Refresh devices
  const refreshDevices = (): void => {
    deviceService.refreshDevices();
  };

  // Control device
  const controlDevice = (deviceId: string, relay: string, state: 'ON' | 'OFF'): void => {
    deviceService.controlDevice(deviceId, relay, state);
  };

  // Context value
  const contextValue: AppContextType = {
    isConnected,
    serverUrl,
    devices,
    setServerUrl,
    connect,
    disconnect,
    refreshDevices,
    controlDevice,
  };

  return (
    <AppContext.Provider value={contextValue}>
      {children}
    </AppContext.Provider>
  );
};

// Custom hook to use the app context
export const useAppContext = () => useContext(AppContext);

// Import for AsyncStorage
import AsyncStorage from '@react-native-async-storage/async-storage';
