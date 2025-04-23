# Bluetooth Low Energy (BLE) Pairing

This document provides detailed information for the backend team on implementing BLE pairing functionality in the mobile app to work with the updated Homato firmware.

## Overview

The Homato firmware now includes BLE support for device setup and WiFi configuration. This allows users to:

1. Discover Homato devices automatically
2. Configure WiFi settings without hardcoding credentials
3. Store multiple network credentials for reliability
4. Pair devices with the Homato ecosystem

## BLE Technical Specifications

### UUIDs

The firmware uses the following UUIDs for BLE services and characteristics:

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Device Info Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8` (Read)
- **WiFi Config Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a9` (Write)

### Pairing Process

1. User presses the BLE button (GPIO 0, usually the BOOT button) for 5 seconds
2. Device enters BLE pairing mode and begins advertising
3. Mobile app scans for devices with the Homato service UUID
4. App connects to the device and reads the device information
5. User enters WiFi credentials in the app
6. App sends credentials to the device
7. Device attempts to connect to WiFi and sends back status
8. On successful WiFi connection, device exits BLE mode and connects to MQTT

## Mobile App Implementation Guide

### 1. Device Discovery

Scan for BLE devices advertising the Homato service UUID:

```javascript
// Example using React Native BLE Plx
import { BleManager } from 'react-native-ble-plx';

const bleManager = new BleManager();
const HOMATO_SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';

const scanForDevices = () => {
  bleManager.startDeviceScan(null, null, (error, device) => {
    if (error) {
      console.error(error);
      return;
    }
    
    if (device.advertisingUUID === HOMATO_SERVICE_UUID) {
      console.log('Homato device found:', device.name);
      setFoundDevices(prevDevices => [...prevDevices, device]);
    }
  });
};
```

### 2. Reading Device Information

Once connected to a device, read the device information characteristic:

```javascript
const getDeviceInfo = async (device) => {
  try {
    await device.connect();
    await device.discoverAllServicesAndCharacteristics();
    
    const info = await device.readCharacteristicForService(
      '4fafc201-1fb5-459e-8fcc-c5c9c331914b',
      'beb5483e-36e1-4688-b7f5-ea07361b26a8'
    );
    
    // The device info is returned as a base64 encoded string
    const deviceInfo = Buffer.from(info.value, 'base64').toString('utf8');
    
    // deviceInfo format: "deviceID:numRelays"
    // e.g. "st-000002:8"
    const [deviceId, relayCount] = deviceInfo.split(':');
    
    return { deviceId, relayCount: parseInt(relayCount, 10) };
  } catch (error) {
    console.error('Failed to read device info:', error);
    throw error;
  }
};
```

### 3. Sending WiFi Credentials

Write the WiFi credentials to the WiFi configuration characteristic:

```javascript
const configureWiFi = async (device, ssid, password) => {
  try {
    // Format: "SSID:password"
    const credentials = `${ssid}:${password}`;
    const data = Buffer.from(credentials).toString('base64');
    
    await device.writeCharacteristicWithResponseForService(
      '4fafc201-1fb5-459e-8fcc-c5c9c331914b',
      'beb5483e-36e1-4688-b7f5-ea07361b26a9',
      data
    );
    
    // Device will attempt to connect and return response
    const response = await device.monitorCharacteristicForService(
      '4fafc201-1fb5-459e-8fcc-c5c9c331914b',
      'beb5483e-36e1-4688-b7f5-ea07361b26a9',
      (error, characteristic) => {
        if (error) {
          console.error('Error monitoring response:', error);
          return;
        }
        
        const responseValue = Buffer.from(characteristic.value, 'base64').toString('utf8');
        console.log('Device response:', responseValue);
        
        if (responseValue.startsWith('Connected to WiFi')) {
          // WiFi configuration successful
          onConfigSuccess(responseValue);
        } else if (responseValue.startsWith('Error')) {
          // WiFi configuration failed
          onConfigError(responseValue);
        }
      }
    );
  } catch (error) {
    console.error('Failed to configure WiFi:', error);
    throw error;
  }
};
```

### 4. Monitoring Connection Status

After sending credentials, monitor the device's response:

```javascript
const onConfigSuccess = (message) => {
  // Extract the local IP address from the success message
  // Format: "Connected to WiFi. IP: 192.168.1.100"
  const ipMatch = message.match(/IP: ([\d\.]+)/);
  const ipAddress = ipMatch ? ipMatch[1] : null;
  
  // Store device information in app state/database
  saveDeviceToDatabase({
    deviceId: currentDevice.deviceId,
    ipAddress: ipAddress,
    relayCount: currentDevice.relayCount,
    name: userProvidedName || `Homato ${currentDevice.deviceId}`,
    lastConnected: new Date().toISOString()
  });
  
  // Navigate to device configuration screen
  navigation.navigate('DeviceSetup', { deviceId: currentDevice.deviceId });
};

const onConfigError = (errorMessage) => {
  // Handle different error cases
  if (errorMessage.includes('Could not connect')) {
    Alert.alert('Connection Failed', 'Could not connect to the WiFi network. Please check the credentials and try again.');
  } else if (errorMessage.includes('Invalid format')) {
    Alert.alert('Invalid Format', 'The WiFi credentials format is invalid. Please try again.');
  } else {
    Alert.alert('Configuration Error', errorMessage);
  }
};
```

### 5. Relay Mapping UI

After successful WiFi configuration, provide a UI for relay mapping:

```javascript
// Example relay mapping component
const RelayMapping = ({ deviceId, relayCount }) => {
  const [relayNames, setRelayNames] = useState(
    Array(relayCount).fill('').map((_, i) => `Relay ${i+1}`)
  );
  
  const saveMapping = async () => {
    // Save relay names to backend
    try {
      const response = await api.saveRelayMapping(deviceId, relayNames);
      if (response.success) {
        Alert.alert('Success', 'Relay mapping saved successfully');
      }
    } catch (error) {
      console.error('Failed to save relay mapping:', error);
      Alert.alert('Error', 'Failed to save relay mapping');
    }
  };
  
  return (
    <View>
      <Text style={styles.title}>Configure Device Relays</Text>
      <Text style={styles.subtitle}>Name each relay according to its connected device</Text>
      
      {relayNames.map((name, index) => (
        <View key={index} style={styles.relayItem}>
          <Text>Relay {index+1}</Text>
          <TextInput
            value={name}
            onChangeText={(text) => {
              const newNames = [...relayNames];
              newNames[index] = text;
              setRelayNames(newNames);
            }}
            placeholder={`Relay ${index+1}`}
            style={styles.input}
          />
        </View>
      ))}
      
      <Button title="Save Mapping" onPress={saveMapping} />
    </View>
  );
};
```

## Testing the BLE Implementation

1. **Device in BLE Mode**: Verify device shows correct status LED pattern (blinking)
2. **Device Discovery**: Confirm device appears in scan results
3. **Connection**: Test establishing connection to the device
4. **Device Info**: Verify correct device ID and relay count is received
5. **WiFi Configuration**: Test sending valid and invalid credentials
6. **Success Handling**: Confirm proper handling of successful configuration
7. **Error Handling**: Test response to invalid credentials and connection failures
8. **Timeout Handling**: Ensure app handles BLE timeouts gracefully
9. **Disconnection**: Test clean disconnection process

## Error Handling

| Error Scenario | Recommended Handling |
|----------------|----------------------|
| Device not found | Prompt user to press BLE button on device |
| Connection fails | Retry connection with exponential backoff |
| Device info unreadable | Disconnect and prompt to reset device |
| WiFi connection timeout | Inform user and suggest checking WiFi signal strength |
| Invalid WiFi credentials | Display error and prompt for correction |
| BLE disconnection during setup | Save progress and offer to resume |

## Security Considerations

1. **Credential Protection**: WiFi credentials should be transmitted only over BLE and never stored unencrypted
2. **BLE Range**: Remind users that BLE has limited range and to keep the device nearby during setup
3. **Pairing Duration**: BLE pairing mode automatically times out after 5 minutes for security
4. **Information Leakage**: Minimize sensitive information in device advertising data

## API Integration

After successful device setup, register the device with the Homato backend:

```javascript
const registerDevice = async (deviceInfo) => {
  try {
    const response = await fetch('https://your-api.com/devices/register', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        deviceId: deviceInfo.deviceId,
        relayCount: deviceInfo.relayCount,
        name: deviceInfo.name,
        userId: currentUserId,
      }),
    });
    
    const data = await response.json();
    return data;
  } catch (error) {
    console.error('Failed to register device:', error);
    throw error;
  }
};
```

## Next Steps

After implementing BLE pairing functionality:

1. Test with multiple device types and firmware versions
2. Create a user-friendly onboarding flow
3. Add device grouping functionality
4. Implement custom relay naming persistence
5. Add device management features (rename, remove, reset)
