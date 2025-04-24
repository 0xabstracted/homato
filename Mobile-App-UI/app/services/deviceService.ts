import socketService from './socketService';

// Types for device data
export interface Relay {
  [key: string]: 'ON' | 'OFF';
}

export interface Device {
  relays: {
    [key: string]: 'ON' | 'OFF';
  };
  online: boolean;
  status: 'online' | 'offline' | 'unreachable';
  lastSeen: number;
}

export interface DeviceRegistry {
  [deviceId: string]: Device;
}

class DeviceService {
  private devices: DeviceRegistry = {};
  private deviceListeners: Set<(devices: DeviceRegistry) => void> = new Set();
  
  constructor() {
    // Listen for device updates from socket service
    socketService.addDeviceUpdateListener(this.handleDeviceUpdate);
    socketService.addDeviceConnectionListener(this.handleDeviceConnectionUpdate);
  }
  
  // Handle initial state and device updates
  private handleDeviceUpdate = (data: any): void => {
    if (data.devices) {
      // Handle initial state with all devices
      this.devices = data.devices;
      this.notifyListeners();
    } else if (data.deviceId && data.relay && data.state) {
      // Handle single relay update
      const { deviceId, relay, state } = data;
      
      if (this.devices[deviceId] && this.devices[deviceId].relays) {
        this.devices[deviceId].relays[relay] = state;
        this.notifyListeners();
      }
    }
  };
  
  // Handle device connection updates
  private handleDeviceConnectionUpdate = (data: any): void => {
    const { deviceId, connected, status } = data;
    
    if (this.devices[deviceId]) {
      this.devices[deviceId].online = connected;
      if (status) {
        this.devices[deviceId].status = status;
      }
      this.notifyListeners();
    }
  };
  
  // Get all devices
  public getDevices(): DeviceRegistry {
    return this.devices;
  }
  
  // Get a specific device
  public getDevice(deviceId: string): Device | null {
    return this.devices[deviceId] || null;
  }
  
  // Control a device relay
  public controlDevice(deviceId: string, relay: string, state: 'ON' | 'OFF'): void {
    socketService.controlDevice(deviceId, relay, state);
    
    // Optimistically update local state
    if (this.devices[deviceId] && this.devices[deviceId].relays) {
      this.devices[deviceId].relays[relay] = state;
      this.notifyListeners();
    }
  }
  
  // Refresh devices from server
  public refreshDevices(): void {
    socketService.getDevices();
  }
  
  // Add device update listener
  public addDeviceListener(listener: (devices: DeviceRegistry) => void): void {
    this.deviceListeners.add(listener);
    // Immediately notify with current state
    listener(this.devices);
  }
  
  // Remove device update listener
  public removeDeviceListener(listener: (devices: DeviceRegistry) => void): void {
    this.deviceListeners.delete(listener);
  }
  
  // Notify all listeners of device updates
  private notifyListeners(): void {
    this.deviceListeners.forEach(listener => {
      try {
        listener(this.devices);
      } catch (error) {
        console.error('Error in device listener:', error);
      }
    });
  }
}

// Create a singleton instance
const deviceService = new DeviceService();

export default deviceService;
