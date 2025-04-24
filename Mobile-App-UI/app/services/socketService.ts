import { io, Socket } from 'socket.io-client';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { 
  DEFAULT_SERVER_URL, 
  SOCKET_OPTIONS, 
  SOCKET_EVENTS 
} from '../config/serverConfig';

// Storage keys
const SERVER_URL_KEY = 'homato_server_url';

class SocketService {
  private socket: Socket | null = null;
  private serverUrl: string = DEFAULT_SERVER_URL;
  private connectionListeners: Set<(connected: boolean) => void> = new Set();
  private deviceUpdateListeners: Set<(data: any) => void> = new Set();
  private deviceConnectionListeners: Set<(data: any) => void> = new Set();
  private errorListeners: Set<(error: any) => void> = new Set();
  
  // Connection status
  private _isConnected: boolean = false;
  
  get isConnected(): boolean {
    return this._isConnected;
  }
  
  // Get current server URL
  public getServerUrl(): string {
    return this.serverUrl;
  }
  
  constructor() {
    this.loadServerUrl();
  }
  
  // Load saved server URL from AsyncStorage
  private async loadServerUrl(): Promise<void> {
    try {
      const savedUrl = await AsyncStorage.getItem(SERVER_URL_KEY);
      if (savedUrl) {
        this.serverUrl = savedUrl;
      }
    } catch (error) {
      console.error('Failed to load server URL:', error);
    }
  }
  
  // Save server URL to AsyncStorage
  public async saveServerUrl(url: string): Promise<void> {
    try {
      await AsyncStorage.setItem(SERVER_URL_KEY, url);
      this.serverUrl = url;
    } catch (error) {
      console.error('Failed to save server URL:', error);
      throw error;
    }
  }
  
  // Initialize socket connection
  public async connect(): Promise<void> {
    // Make sure we have the latest URL
    await this.loadServerUrl();
    
    // Disconnect existing socket if any
    if (this.socket) {
      this.disconnect();
    }
    
    // Create new socket connection
    this.socket = io(this.serverUrl, SOCKET_OPTIONS);
    
    // Set up event listeners
    this.socket.on(SOCKET_EVENTS.CONNECT, () => {
      console.log('Connected to server');
      this._isConnected = true;
      this.notifyConnectionListeners(true);
      
      // Request initial device data
      this.getDevices();
    });
    
    this.socket.on(SOCKET_EVENTS.DISCONNECT, () => {
      console.log('Disconnected from server');
      this._isConnected = false;
      this.notifyConnectionListeners(false);
    });
    
    this.socket.on(SOCKET_EVENTS.ERROR, (error) => {
      console.error('Socket error:', error);
      this.notifyErrorListeners(error);
    });
    
    this.socket.on(SOCKET_EVENTS.INITIAL_STATE, (data) => {
      console.log('Received initial state:', data);
      this.notifyDeviceUpdateListeners(data);
    });
    
    this.socket.on(SOCKET_EVENTS.DEVICE_UPDATE, (data) => {
      console.log('Device update:', data);
      this.notifyDeviceUpdateListeners(data);
    });
    
    this.socket.on(SOCKET_EVENTS.DEVICE_CONNECTION_UPDATE, (data) => {
      console.log('Device connection update:', data);
      this.notifyDeviceConnectionListeners(data);
    });
  }
  
  // Disconnect from server
  public disconnect(): void {
    if (this.socket) {
      this.socket.disconnect();
      this.socket = null;
      this._isConnected = false;
    }
  }
  
  // Request all devices from the server
  public getDevices(): void {
    if (this.socket && this._isConnected) {
      this.socket.emit(SOCKET_EVENTS.GET_DEVICES);
    } else {
      console.warn('Cannot get devices: not connected to server');
    }
  }
  
  // Request a specific device from the server
  public getDevice(deviceId: string): void {
    if (this.socket && this._isConnected) {
      this.socket.emit(SOCKET_EVENTS.GET_DEVICE, { deviceId });
    } else {
      console.warn(`Cannot get device ${deviceId}: not connected to server`);
    }
  }
  
  // Control a device (turn relay on/off)
  public controlDevice(deviceId: string, relay: string, state: 'ON' | 'OFF'): void {
    if (this.socket && this._isConnected) {
      this.socket.emit(SOCKET_EVENTS.CONTROL_DEVICE, {
        deviceId,
        relay,
        state
      });
    } else {
      console.warn(`Cannot control device ${deviceId}: not connected to server`);
    }
  }
  
  // Add connection status listener
  public addConnectionListener(listener: (connected: boolean) => void): void {
    this.connectionListeners.add(listener);
    // Immediately notify with current state
    listener(this._isConnected);
  }
  
  // Remove connection status listener
  public removeConnectionListener(listener: (connected: boolean) => void): void {
    this.connectionListeners.delete(listener);
  }
  
  // Add device update listener
  public addDeviceUpdateListener(listener: (data: any) => void): void {
    this.deviceUpdateListeners.add(listener);
  }
  
  // Remove device update listener
  public removeDeviceUpdateListener(listener: (data: any) => void): void {
    this.deviceUpdateListeners.delete(listener);
  }
  
  // Add device connection listener
  public addDeviceConnectionListener(listener: (data: any) => void): void {
    this.deviceConnectionListeners.add(listener);
  }
  
  // Remove device connection listener
  public removeDeviceConnectionListener(listener: (data: any) => void): void {
    this.deviceConnectionListeners.delete(listener);
  }
  
  // Add error listener
  public addErrorListener(listener: (error: any) => void): void {
    this.errorListeners.add(listener);
  }
  
  // Remove error listener
  public removeErrorListener(listener: (error: any) => void): void {
    this.errorListeners.delete(listener);
  }
  
  // Notify all connection listeners
  private notifyConnectionListeners(connected: boolean): void {
    this.connectionListeners.forEach(listener => {
      try {
        listener(connected);
      } catch (error) {
        console.error('Error in connection listener:', error);
      }
    });
  }
  
  // Notify all device update listeners
  private notifyDeviceUpdateListeners(data: any): void {
    this.deviceUpdateListeners.forEach(listener => {
      try {
        listener(data);
      } catch (error) {
        console.error('Error in device update listener:', error);
      }
    });
  }
  
  // Notify all device connection listeners
  private notifyDeviceConnectionListeners(data: any): void {
    this.deviceConnectionListeners.forEach(listener => {
      try {
        listener(data);
      } catch (error) {
        console.error('Error in device connection listener:', error);
      }
    });
  }
  
  // Notify all error listeners
  private notifyErrorListeners(error: any): void {
    this.errorListeners.forEach(listener => {
      try {
        listener(error);
      } catch (err) {
        console.error('Error in error listener:', err);
      }
    });
  }
}

// Create a singleton instance
const socketService = new SocketService();

export default socketService;
