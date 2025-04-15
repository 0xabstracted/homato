# Troubleshooting Guide

## Common Issues and Solutions

### Device Not Connecting

**Symptoms:**
- Device status shows offline
- Controls not responding
- No MQTT messages being received

**Solutions:**
1. Check WiFi connectivity:
   ```bash
   # On ESP8266
   Serial.println(WiFi.status());  // Should return WL_CONNECTED (3)
   Serial.println(WiFi.localIP()); // Should show valid IP
   ```

2. Verify MQTT credentials:
   - Double-check username/password in `.env`
   - Ensure broker address is correct
   - Verify port number (8883 for SSL)

3. Check device power supply:
   - Measure voltage at ESP8266 (should be 3.3V)
   - Check power adapter output (5V)
   - Inspect USB cable if using USB power

4. Debug connection:
   ```cpp
   // Add to firmware
   #define DEBUG_ESP_PORT Serial
   #define DEBUG_ESP_CORE
   ```

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
   mosquitto_sub -v -h broker -p 8883 -t 'home/#' -u user -P pass
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
   Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
   Serial.printf("Heap fragmentation: %d%%\n", ESP.getHeapFragmentation());
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

1. Factory reset ESP8266:
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

2. Clear MQTT retained messages:
   ```bash
   mosquitto_pub -h broker -p 8883 -t "home/#" -n -r -u user -P pass
   ```

3. Reset web interface:
   - Clear browser data
   - Reload application
   - Reconnect Socket.IO

### Emergency Recovery

1. Upload recovery firmware:
   ```cpp
   // Basic recovery firmware
   void setup() {
     Serial.begin(115200);
     pinMode(LED_BUILTIN, OUTPUT);
     // Set all relay pins to safe state
     for(int i=D1; i<=D8; i++) {
       pinMode(i, OUTPUT);
       digitalWrite(i, HIGH);
     }
   }
   ```

2. Safe mode operation:
   - Boot with minimal configuration
   - Disable automatic features
   - Enable manual control only

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