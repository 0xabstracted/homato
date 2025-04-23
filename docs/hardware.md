# Hardware Setup Guide

## Components Required

### Core Components
- ESP32 Development Board
- 8-Channel Relay Module
- 5V/2A Power Supply
- Jumper Wires
- Project Box
- Terminal Blocks
- DHT11 Temperature/Humidity Sensor
- LDR (Light Dependent Resistor)

### Optional Components
- Status LEDs
- Heat Sinks
- Cooling Fan
- Voltage Regulator
- External Antenna (for better WiFi range)
- Tactile Button (if not using built-in BOOT button for BLE pairing)

## Pin Configuration

### Relay Connections

| Relay Channel | Function      | ESP32 GPIO Pin |
|--------------|---------------|----------------|
| CH1          | Relay 1       | GPIO 23        |
| CH2          | Relay 2       | GPIO 22        |
| CH3          | Relay 3       | GPIO 21        |
| CH4          | Relay 4       | GPIO 19        |
| CH5          | Relay 5       | GPIO 18        |
| CH6          | Relay 6       | GPIO 5         |
| CH7          | Relay 7       | GPIO 25        |
| CH8          | Relay 8       | GPIO 26        |

### Switch Inputs

| Switch       | Function      | ESP32 GPIO Pin |
|--------------|---------------|----------------|
| Switch 1     | Input 1       | GPIO 13        |
| Switch 2     | Input 2       | GPIO 12        |
| Switch 3     | Input 3       | GPIO 14        |
| Switch 4     | Input 4       | GPIO 27        |
| Switch 5     | Input 5       | GPIO 33        |
| Switch 6     | Input 6       | GPIO 32        |
| Switch 7     | Input 7       | GPIO 15        |
| Switch 8     | Input 8       | GPIO 4         |

### Sensors and Control

| Component             | Function          | ESP32 GPIO Pin |
|-----------------------|-------------------|----------------|
| DHT11                 | Temp/Humidity     | GPIO 16        |
| LDR                   | Light Sensor      | GPIO 34        |
| WiFi Status LED       | Connection Status | GPIO 2         |
| BLE Button            | Pairing Mode      | GPIO 0         |

## Wiring Instructions

1. **Power Supply**
   - Connect 5V and GND from power supply to ESP32
   - Connect 5V and GND to relay module VCC and GND
   - Ensure proper voltage regulation
   - ESP32 can be powered via USB during development

2. **Relay Connections**
   - Connect each GPIO pin to corresponding relay input
   - Use pull-up resistors if needed
   - Keep wire lengths minimal
   - Use transistors or optocouplers for isolation when needed

3. **Device Connections**
   - Connect AC devices through terminal blocks
   - Use appropriate gauge wires (at least 16 AWG for power circuits)
   - Label all connections clearly
   - Keep high voltage wiring separate from logic wiring

4. **Sensor Connections**
   - Connect DHT11 data pin to GPIO 16
   - Connect DHT11 VCC to 3.3V and GND to ground
   - Connect LDR to GPIO 34 with appropriate voltage divider
   - Ensure proper placement of sensors for accurate readings

5. **BLE Button**
   - Connect tactile button to GPIO 0 if not using built-in BOOT button
   - Use a pull-up resistor (10k Ω) with the button
   - For the built-in BOOT button, no additional wiring is needed

## Safety Considerations

### Electrical Safety
- Always disconnect power before wiring
- Use proper insulation
- Follow local electrical codes
- Install proper circuit breakers
- Ground all components properly

### Hardware Protection
- Add flyback diodes for inductive loads
- Use fuses for overcurrent protection
- Install surge protectors
- Ensure proper ventilation

## Testing Procedure

1. **Initial Testing**
   - Power on without loads connected
   - Check LED indicators
   - Verify relay clicking sounds
   - Monitor voltage levels
   - Test BLE functionality by pressing the button for 5 seconds

2. **Software Testing**
   - Upload test firmware
   - Verify each relay triggers
   - Check status feedback
   - Test MQTT communication
   - Test BLE pairing and WiFi configuration
   - Verify sensor readings

3. **Load Testing**
   - Connect one device at a time
   - Monitor temperature
   - Check for interference
   - Verify state persistence

## Troubleshooting

### Common Hardware Issues

1. **Relay Not Triggering**
   - Check GPIO connections
   - Verify power supply
   - Test relay module separately
   - Check for loose connections
   - Verify GPIO pin assignments in firmware

2. **Device Not Responding**
   - Verify wiring polarity
   - Check relay contacts
   - Test power delivery
   - Inspect fuses
   - Verify USB connection during programming

3. **System Instability**
   - Check power supply capacity
   - Monitor voltage levels
   - Verify ground connections
   - Check for interference
   - Ensure firmware uses the huge_app partition scheme
   - Check for memory leaks in firmware

4. **BLE Not Working**
   - Verify button connection
   - Check if button press is being detected (LED should blink)
   - Try resetting the device
   - Verify BLE service is running on mobile device

5. **Sensor Reading Issues**
   - Check sensor connections
   - Verify sensor is powered correctly
   - Test sensor with a simple test sketch
   - Check sensor placement for accurate readings

## Maintenance

### Regular Checks
- Inspect connections monthly
- Clean dust accumulation
- Check for loose screws
- Monitor temperature
- Test safety systems

### Replacement Schedule
- Relay contacts: 2-3 years
- Power supply: 3-4 years
- Cooling fan: 2 years
- Terminal blocks: As needed
- DHT sensor: 1-2 years (depending on environment)
- ESP32 module: As needed (typically 5+ years)

## Housing and Mounting

### Enclosure Requirements
- Use a properly ventilated enclosure
- Allow space for heat dissipation
- Include access for the BLE button
- Consider transparent cover to view LEDs
- Keep high-voltage components isolated

### Installation Location
- Mount in a clean, dry location
- Avoid extreme temperatures
- Consider accessibility for maintenance
- Allow space for cable management
- Keep away from water sources and humidity 