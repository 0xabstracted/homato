# Troubleshooting Guide

This guide covers common issues and their solutions for the Homato Home Automation System, including firmware, hardware, backend, and BLE configuration problems.

## Common Issues and Solutions

### Device Not Connecting

**Symptoms:**
- Device status shows offline
- Controls not responding
- No MQTT messages being received

**Solutions:**
1. Check WiFi connectivity:
   ```bash
   # On ESP32
   Serial.println(WiFi.status());  // Should return WL_CONNECTED (3)
   Serial.println(WiFi.localIP()); // Should show valid IP
   Serial.println(WiFi.RSSI());    // Signal strength in dBm
   ```

2. Verify MQTT credentials:
   - Double-check username/password in `.env`
   - Ensure broker address is correct
   - Verify port number (8883 for SSL)

3. Check device power supply:
   - Measure voltage at ESP32 (should be 3.3V)
   - Check power adapter output (5V)
   - Inspect USB cable if using USB power
   - Ensure sufficient power for all relays (2A minimum)

4. Debug connection:
   ```cpp
   // Add to firmware
   #define CORE_DEBUG_LEVEL 3
   #include "esp_log.h"
   
   // Set the log level for WiFi component
   esp_log_level_set("wifi", ESP_LOG_VERBOSE);
   ```

5. Test multiple WiFi credentials:
   - Check if the device is connecting to alternative networks
   - Verify stored credentials with BLE connection
   - Try to add a new WiFi network via BLE

### MQTT Connection Failed

**Symptoms:**
- "Connection failed" in logs
- Intermittent connectivity
- High latency in device response

**Solutions:**
1. Check MQTT broker status:
   ```bash
   # Test broker connection
   mosquitto_sub -h your-broker -p 8883 -t "test" -u username -P password --cafile ca.crt
   ```

2. Verify SSL/TLS settings:
   - Ensure certificates are valid
   - Check certificate expiration
   - Verify broker supports SSL/TLS

3. Network troubleshooting:
   - Check firewall rules
   - Verify port forwarding
   - Test network latency

### Relay Not Responding

**Symptoms:**
- UI shows ON but device remains OFF
- Delayed response
- Inconsistent behavior

**Solutions:**
1. Check wiring:
   - Verify GPIO connections
   - Test continuity
   - Check for loose connections

2. Test relay module:
   ```cpp
   // Test code
   void testRelay() {
     digitalWrite(relayPin, LOW);  // ON
     delay(1000);
     digitalWrite(relayPin, HIGH); // OFF
     delay(1000);
   }
   ```

3. Verify power supply:
   - Check relay module voltage
   - Measure current draw
   - Test with different power source

### Web Interface Issues

**Symptoms:**
- UI not updating
- Buttons not responding
- Connection status errors

**Solutions:**
1. Clear browser cache:
   - Clear cookies and cache
   - Try incognito mode
   - Test different browser

2. Check Socket.IO connection:
   ```javascript
   // Add to frontend code
   socket.on('connect_error', (error) => {
     console.error('Connection Error:', error);
   });
   ```

3. Debug server logs:
   ```bash
   DEBUG=* npm start
   ```

## ESP32 Specific Issues

### Compilation Errors

**Symptoms:**
- "Sketch too large" error
- Out of memory errors
- Compilation fails

**Solutions:**
1. Use the huge_app partition scheme:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app,FlashMode=qio,FlashFreq=80,UploadSpeed=921600 homato_v1.ino
   ```

2. Optimize memory usage:
   ```cpp
   // Reduce string literals
   const char* const PROGMEM topics[] = {"relay1", "relay2"};
   
   // Minimize Serial output
   #ifdef DEBUG_MODE
     Serial.println("Debug message");
   #endif
   ```

3. Check library compatibility:
   - Verify BLE and WiFi libraries are compatible
   - Remove unused libraries
   - Use ESP32-specific versions of libraries

### BLE Pairing Issues

**Symptoms:**
- Device not discoverable
- Connection drops during pairing
- WiFi configuration fails

**Solutions:**
1. Check BLE activation:
   ```cpp
   // Test BLE activation with LED feedback
   void testBLEButton() {
     pinMode(BLE_BUTTON_PIN, INPUT_PULLUP);
     pinMode(BLE_LED_PIN, OUTPUT);
     
     if (!digitalRead(BLE_BUTTON_PIN)) {
       digitalWrite(BLE_LED_PIN, HIGH);
       Serial.println("BLE button pressed");
     } else {
       digitalWrite(BLE_LED_PIN, LOW);
     }
   }
   ```

2. Debug BLE services:
   ```cpp
   Serial.println("BLE Device MAC: " + BLEDevice::getAddress().toString());
   Serial.println("BLE Device Name: " + String(BLEDevice::getName().c_str()));
   ```

3. Test BLE connection with a generic app:
   - Use nRF Connect or similar app
   - Verify service UUID is visible
   - Test reading device info characteristic
   - Test writing to WiFi config characteristic

4. Reset BLE state:
   ```cpp
   BLEDevice::deinit(true);
   delay(500);
   // Then reinitialize BLE
   ```

### WiFi Credential Storage Issues

**Symptoms:**
- Device forgets WiFi credentials after restart
- Cannot store multiple networks
- EEPROM corruption

**Solutions:**
1. Debug EEPROM reading/writing:
   ```cpp
   void printStoredCredentials() {
     for (int i = 0; i < 3; i++) {
       Serial.print("Credential slot ");
       Serial.print(i);
       Serial.print(": Valid=");
       Serial.print(storedCredentials[i].valid);
       Serial.print(", SSID=");
       Serial.println(storedCredentials[i].ssid);
     }
   }
   ```

2. Reset credential storage:
   ```cpp
   void resetWiFiCredentials() {
     for (int i = 0; i < 3; i++) {
       storedCredentials[i].valid = false;
       storedCredentials[i].ssid[0] = '\0';
       storedCredentials[i].password[0] = '\0';
     }
     EEPROM.begin(EEPROM_SIZE);
     EEPROM.write(WIFI_CONFIG_INITIALIZED_ADDR, 0);
     EEPROM.commit();
   }
   ```

3. Test with known good credentials:
   - Hard-code test credentials temporarily
   - Verify against a WiFi network with simple settings
   - Check for special characters in SSID/password

## Advanced Debugging

### Backend Debugging

1. Enable detailed logging:
   ```javascript
   const debug = require('debug')('app:*');
   debug('Detailed state:', JSON.stringify(deviceState, null, 2));
   ```

2. Monitor MQTT traffic:
   ```bash
   # Subscribe to all topics
   mosquitto_sub -v -h broker -p 8883 -t '#' -u user -P pass
   ```

   Or for a specific device:
   ```bash
   mosquitto_sub -v -h broker -p 8883 -t 'st-000002/#' -u user -P pass
   ```

3. Check system resources:
   ```bash
   # Monitor CPU and memory
   top -p $(pgrep -f 'node app.js')
   ```

### Firmware Debugging

1. Enable serial debugging:
   ```cpp
   #define MQTT_DEBUG       // MQTT debugging
   #define WIFI_DEBUG       // WiFi debugging
   Serial.begin(115200);    // Initialize serial
   ```

2. Monitor memory usage:
   ```cpp
   Serial.printf("ESP32 Free heap: %d bytes\n", ESP.getFreeHeap());
   Serial.printf("ESP32 Total heap: %d bytes\n", ESP.getHeapSize());
   Serial.printf("ESP32 Used heap: %d bytes\n", ESP.getHeapSize() - ESP.getFreeHeap());
   Serial.printf("ESP32 Min free heap: %d bytes\n", ESP.getMinFreeHeap());
   Serial.printf("ESP32 Max alloc heap: %d bytes\n", ESP.getMaxAllocHeap());
   ```

3. Test GPIO outputs:
   ```cpp
   // GPIO test function
   void testGPIO(int pin) {
     pinMode(pin, OUTPUT);
     for(int i=0; i<5; i++) {
       digitalWrite(pin, !digitalRead(pin));
       delay(500);
     }
   }
   ```

## Recovery Procedures

### System Reset

1. Factory reset ESP32:
   ```cpp
   void factoryReset() {
     EEPROM.begin(512);
     for(int i=0; i<512; i++) {
       EEPROM.write(i, 0);
     }
     EEPROM.commit();
     ESP.restart();
   }
   ```

   Alternatively, use esptool to completely erase flash:
   ```bash
   python3 -m esptool --chip esp32 --port /dev/cu.usbserial-0001 erase_flash
   ```

2. Clear MQTT retained messages:
   ```bash
   # Clear all retained messages for a specific device
   mosquitto_pub -h broker -p 8883 -t "st-000002/#" -n -r -u user -P pass
   
   # Or clear retained messages for all devices
   mosquitto_pub -h broker -p 8883 -t "#" -n -r -u user -P pass
   ```

3. Reset web interface:
   - Clear browser data
   - Reload application
   - Reconnect Socket.IO

### Emergency Recovery

1. Upload recovery firmware:
   ```cpp
   // Basic recovery firmware for ESP32
   void setup() {
     Serial.begin(115200);
     pinMode(2, OUTPUT); // Built-in LED on most ESP32 boards
     
     // Set all relay pins to safe state (OFF)
     int relayPins[] = {23, 22, 21, 19, 18, 5, 25, 26};
     for(int i=0; i<8; i++) {
       pinMode(relayPins[i], OUTPUT);
       digitalWrite(relayPins[i], HIGH); // HIGH = OFF for most relay modules
     }
     
     // Blink LED to indicate recovery mode
     for(int i=0; i<10; i++) {
       digitalWrite(2, !digitalRead(2));
       delay(200);
     }
     
     Serial.println("RECOVERY MODE ACTIVE");
   }
   
   void loop() {
     // Blink pattern to indicate recovery mode
     digitalWrite(2, HIGH);
     delay(2000);
     digitalWrite(2, LOW);
     delay(200);
   }
   ```

2. Safe mode operation:
   - Boot with minimal configuration
   - Disable automatic features
   - Enable manual control only

## Multi-Device Troubleshooting

### Device ID Conflicts

**Symptoms:**
- Multiple devices responding to same commands
- Inconsistent device behavior
- MQTT topic conflicts

**Solutions:**
1. Verify unique device IDs:
   ```cpp
   // In setup(), print the device ID
   Serial.print("Device ID: ");
   Serial.println(DEVICE_ID);
   ```

2. Check MQTT subscriptions:
   ```bash
   # List all connected clients
   mosquitto_sub -v -h broker -p 8883 -t '$SYS/broker/clients/#' -u user -P pass
   ```

3. Monitor topic activity:
   ```bash
   # See which device responds to which topic
   mosquitto_sub -v -h broker -p 8883 -t '#' -u user -P pass
   ```

### Backend-Multiple Device Issues

**Symptoms:**
- UI shows incorrect device states
- Commands sent to wrong device
- Device list inconsistencies

**Solutions:**
1. Debug device registry:
   ```javascript
   // Print the device registry
   console.log('Device Registry:', JSON.stringify(deviceRegistry, null, 2));
   ```

2. Check topic subscriptions:
   ```javascript
   // Log all MQTT subscriptions
   mqttClient.getSubscriptions().forEach(sub => {
     console.log(`Subscribed to: ${sub.topic} with QoS ${sub.qos}`);
   });
   ```

3. Test device-specific commands:
   ```javascript
   // Test publishing to specific device
   mqttClient.publish(`st-000002/relay1`, 'ON', { retain: true, qos: 1 });
   ```

## Maintenance Tips

1. Regular checks:
   - Monitor system logs
   - Check device temperatures
   - Verify relay operations
   - Test safety features

2. Backup procedures:
   - Export configuration
   - Backup firmware
   - Document changes
   - Store credentials safely

3. Update procedures:
   - Test updates in development
   - Maintain rollback capability
   - Schedule maintenance windows
   - Monitor post-update behavior
   - Always use the huge_app partition scheme
   - Backup BLE and WiFi configuration 