# Getting Started with Homato

## Prerequisites

Before you begin, ensure you have:

- Node.js v14+
- Arduino IDE
- ESP8266 board
- Relay module
- Basic electronics tools

## Quick Start

1. **Clone the Repository**
   ```bash
   git clone https://github.com/0xabstracted/homato.git
   cd homato
   ```

2. **Set Up the Backend**
   ```bash
   cd homato-control-system
   npm install
   ```

3. **Configure Environment**
   - Copy `.env.example` to `.env`
   - Update MQTT credentials
   ```bash
   cp .env.example .env
   # Edit .env with your preferred editor
   ```

4. **Start the Server**
   ```bash
   npm start
   ```
   
   Or using Docker:
   ```bash
   sudo docker build -t app .
   sudo docker run -d -it -p 3000:3000 --name appc app
   ```

5. **Access the Interface**
   - Open `http://localhost:3000` in your browser
   - The interface should show the device control panel

## Development Setup

1. **Install Development Dependencies**
   ```bash
   npm install --save-dev nodemon
   ```

2. **Run in Development Mode**
   ```bash
   npm run dev
   ```

3. **Enable Debug Logging**
   ```bash
   DEBUG=* npm run dev
   ```

## Next Steps

- Read the [Installation Guide](installation.md) for detailed setup instructions
- Follow the [Hardware Setup Guide](hardware.md) for physical device configuration
- Check [Configuration Options](configuration.md) for customization
- Review [Security Considerations](security.md) before deployment

## Common Issues

- If the web interface isn't accessible, check if the server is running
- For MQTT connection issues, verify your credentials in `.env`
- Hardware connection problems? See [Troubleshooting](troubleshooting.md) 