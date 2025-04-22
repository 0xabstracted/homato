# Homato Development Guide

## Build & Run Commands
- `npm start` - Start the application
- `npm run dev` - Run in development mode with auto-reload
- `npm test` - Run tests (currently not implemented)

## Code Style Guidelines

### JavaScript
- Use ES6+ features and syntax
- Follow standard camelCase naming convention
- Use meaningful variable/function names
- Comment complex logic with clear explanations
- Import order: built-in modules, external packages, local modules
- Error handling: Use try/catch blocks for async operations
- Properly document functions, parameters, and return values

### Firmware (Arduino)
- Follow Arduino coding style
- Document hardware connections with pin numbers
- Add comments for complex logic sections
- Use consistent naming conventions for variables
- Save state changes to EEPROM for persistence

## Linting and Formatting
- Keep code consistent with existing style
- Maintain clean indentation (2 spaces)
- Use semicolons for statement termination
- Avoid magic numbers - use constants with descriptive names

## Architecture Guidelines
- Separate concerns between frontend/backend
- Use MQTT for device communication
- Handle device connection status appropriately
- Implement proper error handling and logging
- Maintain responsive design for UI components