// Connect to Socket.io
const socket = io();

// DOM Elements
const connectionIndicator = document.getElementById('connection-indicator');
const deviceConnectionStatus = document.getElementById('device-connection-status');
const switchControl = document.getElementById('switch-control');
const lightControl = document.getElementById('light-control');
const switchStatus = document.getElementById('switch-status');
const lightStatus = document.getElementById('light-status');
const switchCard = document.getElementById('switch-card');
const lightCard = document.getElementById('light-card');
const logEntries = document.getElementById('log-entries');

// Device connection state
let deviceConnected = false;

// Initialize UI state
switchControl.checked = false;
lightControl.checked = false;
updateStatusIndicator(switchStatus, 'Unknown');
updateStatusIndicator(lightStatus, 'Unknown');
updateDeviceConnectionStatus(false);

// Socket connection status
socket.on('connect', () => {
    connectionIndicator.textContent = 'Connected';
    connectionIndicator.className = 'connected';
    addLogEntry('Connected to server');
});

socket.on('disconnect', () => {
    connectionIndicator.textContent = 'Disconnected';
    connectionIndicator.className = 'disconnected';
    addLogEntry('Disconnected from server', 'error');
    
    // When server disconnects, also mark device as disconnected
    updateDeviceConnectionStatus(false);
});

// Initial state from server
socket.on('initialState', (data) => {
    updateUIState(data);
    
    // If we have device connection info, update it
    if (data.hasOwnProperty('deviceConnected')) {
        updateDeviceConnectionStatus(data.deviceConnected);
    }
    
    addLogEntry('Received initial device state');
});

// Device update events
socket.on('deviceUpdate', (data) => {
    if (data.topic.includes('switch')) {
        updateStatus('switch', data.state);
        addLogEntry(`Switch state updated to: ${data.state}`, data.state === 'ON' ? 'success' : '');
    } else if (data.topic.includes('light')) {
        updateStatus('light', data.state);
        addLogEntry(`Light state updated to: ${data.state}`, data.state === 'ON' ? 'success' : '');
    }
});

// Device connection update events
socket.on('deviceConnectionUpdate', (data) => {
    updateDeviceConnectionStatus(data.connected);
    addLogEntry(
        `Device is now ${data.connected ? 'connected' : 'disconnected'}`, 
        data.connected ? 'success' : 'error'
    );
});

// Control event listeners
switchControl.addEventListener('change', () => {
    if (!deviceConnected) {
        // Prevent toggle if device is offline
        switchControl.checked = !switchControl.checked;
        addLogEntry('Cannot control switch: Device is offline', 'error');
        return;
    }
    
    const state = switchControl.checked ? 'ON' : 'OFF';
    socket.emit('controlDevice', { device: 'switch', state });
    addLogEntry(`Requested switch change to: ${state}`);
});

lightControl.addEventListener('change', () => {
    if (!deviceConnected) {
        // Prevent toggle if device is offline
        lightControl.checked = !lightControl.checked;
        addLogEntry('Cannot control light: Device is offline', 'error');
        return;
    }
    
    const state = lightControl.checked ? 'ON' : 'OFF';
    socket.emit('controlDevice', { device: 'light', state });
    addLogEntry(`Requested light change to: ${state}`);
});

// Helper functions
function updateUIState(data) {
    if (data.switch) {
        updateStatus('switch', data.switch);
    }
    
    if (data.light) {
        updateStatus('light', data.light);
    }
}

function updateStatus(device, state) {
    const isOn = state === 'ON';
    
    if (device === 'switch') {
        switchControl.checked = isOn;
        updateStatusIndicator(switchStatus, state);
    } else if (device === 'light') {
        lightControl.checked = isOn;
        updateStatusIndicator(lightStatus, state);
    }
}

function updateStatusIndicator(element, state) {
    element.textContent = state;
    element.className = '';
    
    if (state === 'ON') {
        element.classList.add('status-on');
    } else if (state === 'OFF') {
        element.classList.add('status-off');
    }
}

function updateDeviceConnectionStatus(connected) {
    // Update global state
    deviceConnected = connected;
    
    // Update status indicator
    if (deviceConnectionStatus) {
        deviceConnectionStatus.textContent = connected ? 'Connected' : 'Disconnected';
        deviceConnectionStatus.className = connected ? 'connected' : 'disconnected';
    }
    
    // Update device cards
    switchCard.classList.toggle('disabled', !connected);
    lightCard.classList.toggle('disabled', !connected);
    
    // Enable/disable controls based on connection
    switchControl.disabled = !connected;
    lightControl.disabled = !connected;
}

function addLogEntry(message, type = '') {
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    
    if (type) {
        entry.classList.add(type);
    }
    
    const timestamp = document.createElement('span');
    timestamp.className = 'log-timestamp';
    timestamp.textContent = new Date().toLocaleTimeString();
    
    entry.appendChild(timestamp);
    entry.appendChild(document.createTextNode(message));
    
    logEntries.appendChild(entry);
    logEntries.scrollTop = logEntries.scrollHeight;
    
    // Limit number of log entries to prevent performance issues
    const maxEntries = 50;
    while (logEntries.children.length > maxEntries) {
        logEntries.removeChild(logEntries.firstChild);
    }
}

// Fetch initial state if not received from socket
setTimeout(() => {
    if (switchStatus.textContent === 'Unknown') {
        fetch('/api/status')
            .then(response => response.json())
            .then(data => {
                updateUIState(data.deviceState);
                if (data.deviceState.hasOwnProperty('deviceConnected')) {
                    updateDeviceConnectionStatus(data.deviceState.deviceConnected);
                }
                addLogEntry('Fetched device state via API');
            })
            .catch(error => {
                addLogEntry('Error fetching device state: ' + error.message, 'error');
            });
    }
}, 3000);