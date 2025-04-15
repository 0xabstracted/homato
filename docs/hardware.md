# Hardware Setup Guide

## Components Required

### Core Components
- ESP8266 NodeMCU
- 8-Channel Relay Module
- 5V/2A Power Supply
- Jumper Wires
- Project Box
- Terminal Blocks

### Optional Components
- Status LEDs
- Heat Sinks
- Cooling Fan
- Voltage Regulator

## Pin Configuration

| Device        | GPIO Pin | NodeMCU Pin | Relay Channel |
|--------------|----------|-------------|---------------|
| Main Switch  | GPIO5    | D1         | CH1           |
| Light        | GPIO4    | D2         | CH2           |
| Fan          | GPIO0    | D3         | CH3           |
| Tube Light   | GPIO2    | D4         | CH4           |
| Bed Light    | GPIO14   | D5         | CH5           |
| False Ceiling| GPIO12   | D6         | CH6           |
| AC           | GPIO13   | D7         | CH7           |
| Switch Port  | GPIO15   | D8         | CH8           |

## Wiring Instructions

1. **Power Supply**
   - Connect 5V and GND from power supply to NodeMCU
   - Connect 5V and GND to relay module VCC and GND
   - Ensure proper voltage regulation

2. **Relay Connections**
   - Connect each GPIO pin to corresponding relay input
   - Use pull-up resistors if needed
   - Keep wire lengths minimal

3. **Device Connections**
   - Connect AC devices through terminal blocks
   - Use appropriate gauge wires
   - Label all connections clearly

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

2. **Software Testing**
   - Upload test firmware
   - Verify each relay triggers
   - Check status feedback
   - Test MQTT communication

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

2. **Device Not Responding**
   - Verify wiring polarity
   - Check relay contacts
   - Test power delivery
   - Inspect fuses

3. **System Instability**
   - Check power supply capacity
   - Monitor voltage levels
   - Verify ground connections
   - Check for interference

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