# Frequently Asked Questions (FAQ)

## General Questions

### What is Homato?
Homato is an open-source home automation system that allows you to control various household devices through a web interface. It uses ESP8266 microcontrollers and MQTT protocol for communication.

### What devices can I control with Homato?
Currently supported devices include:
- Main power switch
- Lights (regular, tube, bed, false ceiling)
- Fans
- Air conditioners
- Switch ports (general purpose)

### Is Homato secure?
Yes, Homato implements several security measures:
- Secure MQTT communication (SSL/TLS)
- Password protection
- State persistence
- Configurable access controls

### Can I use Homato without internet?
Yes, you can set up a local MQTT broker (like Mosquitto) to run Homato entirely on your local network.

## Installation

### What do I need to get started?
Minimum requirements:
- ESP8266 NodeMCU
- 8-Channel Relay Module
- Power supply
- Basic electronics tools
- Computer with Node.js installed

### Why am I getting "Connection Failed" errors?
Common reasons include:
1. Incorrect MQTT credentials
2. Firewall blocking ports
3. Wrong broker address
4. Network connectivity issues

### How do I update the firmware?
1. Download the latest firmware
2. Open Arduino IDE
3. Load the new firmware
4. Upload to ESP8266
5. Device will restart automatically

## Hardware

### Which ESP8266 board should I use?
We recommend NodeMCU v1.0 because:
- Built-in USB programmer
- Sufficient GPIO pins
- 3.3V regulation
- Compact size

### Can I use a different relay module?
Yes, any relay module that:
- Operates at 5V
- Has opto-isolation
- Can handle your load current
- Has enough channels

### What's the maximum power rating?
Depends on your relay module, typically:
- AC: 250V/10A per channel
- DC: 30V/10A per channel
Always check your specific relay's specifications.

## Software

### Can I customize the web interface?
Yes, you can modify:
- UI layout (`public/index.html`)
- Styling (`public/styles.css`)
- Functionality (`public/app.js`)

### How do I add new devices?
1. Add device in firmware:
   ```cpp
   const int newDevicePin = D5;
   ```
2. Add MQTT topic:
   ```javascript
   const mqtt_topic_newdevice = "home/newdevice";
   ```
3. Update web interface

### Why are my devices showing as offline?
Check:
1. Power supply to ESP8266
2. WiFi connection
3. MQTT broker connection
4. Device availability topic

## Troubleshooting

### Device states reset after power loss
Ensure:
1. EEPROM is properly initialized
2. States are being saved
3. Recovery code is working
```cpp
void saveState() {
    EEPROM.write(STATE_ADDR, currentState);
    EEPROM.commit();
}
```

### High latency in device response
Solutions:
1. Check network quality
2. Reduce MQTT QoS level
3. Optimize WiFi settings
4. Use local MQTT broker

### Multiple devices switching simultaneously
Causes:
1. Ground loop issues
2. Power supply problems
3. Interference
Fix by:
- Isolating power supplies
- Adding delay between switches
- Checking wiring

## Development

### How do I contribute to Homato?
1. Fork the repository
2. Create feature branch
3. Make changes
4. Submit pull request
See [Contributing Guide](CONTRIBUTING.md)

### Can I add my own features?
Yes! Common additions:
- New device types
- Automation rules
- Custom schedules
- Additional sensors

### How do I debug issues?
1. Enable debug logging:
   ```javascript
   DEBUG=* npm start
   ```
2. Monitor MQTT messages:
   ```bash
   mosquitto_sub -v -t 'home/#'
   ```
3. Use Serial monitoring for ESP8266

## Integration

### Can I integrate with Home Assistant?
Yes, through:
1. MQTT integration
2. REST API
3. Custom components

### Does it work with other smart home systems?
Compatible with:
- Home Assistant
- OpenHAB
- Node-RED
- Custom systems via MQTT

### Can I control it with my phone?
Yes, through:
1. Web interface (mobile-responsive)
2. Custom mobile apps (via API)
3. Home automation apps

## Scaling

### How many devices can I control?
Limits:
- ESP8266: 8 devices per board
- MQTT: No practical limit
- System: Tested with 50+ devices

### Can I use multiple ESP8266 boards?
Yes:
1. Use unique client IDs
2. Configure different GPIO pins
3. Update topic structure

### How do I handle multiple rooms?
Strategies:
1. Separate ESP8266 per room
2. Topic structure: `home/room/device`
3. Group devices in UI

## Maintenance

### How often should I check the system?
Recommended schedule:
- Weekly: Check connections
- Monthly: Update firmware
- Quarterly: Clean hardware
- Yearly: Replace worn components

### What regular maintenance is needed?
1. Check relay contacts
2. Update software
3. Monitor logs
4. Backup configuration

### How do I backup the system?
Backup:
1. Configuration files
2. Firmware code
3. MQTT settings
4. Hardware documentation 