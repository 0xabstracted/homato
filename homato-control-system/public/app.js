// Connect to Socket.io
const socket = io();

// DOM Elements
const connectionIndicator = document.getElementById('connection-indicator');
const deviceConnectionStatus = document.getElementById('device-connection-status');
const deviceSelector = document.getElementById('device-selector');
const relayGrid = document.getElementById('relay-grid');
const currentDeviceId = document.getElementById('current-device-id');
const deviceLastSeen = document.getElementById('device-last-seen');
const logEntries = document.getElementById('log-entries');
const relayCardTemplate = document.getElementById('relay-card-template');

// Store devices data
let devices = {};
let currentDevice = null;
const RELAYS_PER_DEVICE = 8;

// Socket connection status
socket.on('connect', () => {
    connectionIndicator.textContent = 'Connected';
    connectionIndicator.className = 'connected';
    addLogEntry('Connected to server');
    
    // When reconnecting, fetch device data again
    socket.emit('getDevices');
});

socket.on('disconnect', () => {
    connectionIndicator.textContent = 'Disconnected';
    connectionIndicator.className = 'disconnected';
    addLogEntry('Disconnected from server', 'error');

    // Update all device statuses to disconnected
    updateAllDevicesStatus(false);
});

// Initial state from server
socket.on('initialState', (data) => {
    if (data.devices) {
        devices = data.devices;
        updateDeviceSelector();
        
        // If we have devices, select the first one by default
        const deviceIds = Object.keys(devices);
        if (deviceIds.length > 0) {
            selectDevice(deviceIds[0]);
        }
        
        addLogEntry(`Received state for ${deviceIds.length} devices`);
    }
});

// Device update events
socket.on('deviceUpdate', (data) => {
    const { deviceId, relay, state } = data;
    
    // Update our local state
    if (devices[deviceId] && devices[deviceId].relays) {
        devices[deviceId].relays[relay] = state;
        
        // If this is the currently selected device, update UI
        if (currentDevice === deviceId) {
            updateRelayStatus(relay, state);
        }
        
        addLogEntry(`${deviceId} ${relay} updated to: ${state}`,
            state === 'ON' ? 'success' : '');
    }
});

// Device connection update events
socket.on('deviceConnectionUpdate', (data) => {
    const { deviceId, connected, status } = data;
    
    // Update our local state
    if (devices[deviceId]) {
        devices[deviceId].online = connected;
        devices[deviceId].status = status || (connected ? 'online' : 'offline');
        
        // If this is the currently selected device, update UI
        if (currentDevice === deviceId) {
            updateDeviceStatus(deviceId);
        }
        
        // Add log entry with appropriate message based on status
        let statusMessage = connected ? 'connected' : 'disconnected';
        if (status === 'unreachable') {
            statusMessage = 'unreachable (inactive for over 1 hour)';
        }
        
        addLogEntry(
            `Device ${deviceId} is now ${statusMessage}`,
            connected ? 'success' : (status === 'unreachable' ? 'warning' : 'error')
        );
    }
});

// Handle any errors from the server
socket.on('error', (data) => {
    addLogEntry(data.message, 'error');
});

// Device selector handler
deviceSelector.addEventListener('change', function() {
    const deviceId = this.value;
    if (deviceId) {
        selectDevice(deviceId);
    } else {
        clearRelayGrid();
        currentDevice = null;
        currentDeviceId.textContent = 'None';
        deviceConnectionStatus.textContent = 'Unknown';
        deviceConnectionStatus.className = '';
        deviceLastSeen.textContent = 'Never';
    }
});

// Function to select a device and display its relays
function selectDevice(deviceId) {
    if (!devices[deviceId]) return;
    
    currentDevice = deviceId;
    currentDeviceId.textContent = deviceId;
    
    // Update device status indicators
    updateDeviceStatus(deviceId);
    
    // Create relay cards for this device
    createRelayCards(deviceId);
}

// Function to update device status indicators
function updateDeviceStatus(deviceId) {
    if (!devices[deviceId]) return;
    
    const device = devices[deviceId];
    const isOnline = device.online;
    const status = device.status || (isOnline ? 'online' : 'offline');
    
    // Update connection status with more detailed information
    let statusText = 'Unknown';
    let statusClass = '';
    
    if (status === 'online') {
        statusText = 'Connected';
        statusClass = 'connected';
    } else if (status === 'offline') {
        statusText = 'Disconnected';
        statusClass = 'disconnected';
    } else if (status === 'unreachable') {
        statusText = 'Unreachable';
        statusClass = 'unreachable';
    }
    
    deviceConnectionStatus.textContent = statusText;
    deviceConnectionStatus.className = statusClass;
    
    // Update last seen timestamp
    const lastSeen = new Date(device.lastSeen);
    deviceLastSeen.textContent = lastSeen.toLocaleString();
    
    // Enable/disable all relay controls based on device status
    const relayControls = document.querySelectorAll('.relay-card input[type="checkbox"]');
    relayControls.forEach(control => {
        control.disabled = !isOnline;
    });
    
    // Add/remove disabled class from relay cards
    const relayCards = document.querySelectorAll('.relay-card');
    relayCards.forEach(card => {
        card.classList.toggle('disabled', !isOnline);
        
        // Add specific class for unreachable devices
        if (status === 'unreachable') {
            card.classList.add('unreachable');
        } else {
            card.classList.remove('unreachable');
        }
    });
}

// Function to mark all devices as offline when server disconnects
function updateAllDevicesStatus(isOnline) {
    Object.keys(devices).forEach(deviceId => {
        devices[deviceId].online = isOnline;
        if (deviceId === currentDevice) {
            updateDeviceStatus(deviceId);
        }
    });
}

// Function to update device selector dropdown
function updateDeviceSelector() {
    // Clear existing options except the default one
    while (deviceSelector.options.length > 1) {
        deviceSelector.remove(1);
    }
    
    // Add options for each device
    const deviceIds = Object.keys(devices).sort();
    if (deviceIds.length === 0) {
        deviceSelector.options[0].text = 'No devices available';
    } else {
        deviceSelector.options[0].text = 'Select a device';
        
        deviceIds.forEach(deviceId => {
            const option = document.createElement('option');
            option.value = deviceId;
            option.text = deviceId;
            deviceSelector.appendChild(option);
        });
    }
}

// Function to create relay cards for a device
function createRelayCards(deviceId) {
    clearRelayGrid();
    
    if (!devices[deviceId] || !devices[deviceId].relays) return;
    
    const device = devices[deviceId];
    const isOnline = device.online;
    
    // For each relay, create a card
    for (let i = 1; i <= RELAYS_PER_DEVICE; i++) {
        const relayId = `relay${i}`;
        if (device.relays.hasOwnProperty(relayId)) {
            const state = device.relays[relayId];
            
            // Clone the template content
            const template = relayCardTemplate.innerHTML;
            let cardHtml = template
                .replace(/{relayId}/g, relayId)
                .replace(/{relayNumber}/g, i);
            
            // Create temporary container to insert HTML
            const temp = document.createElement('div');
            temp.innerHTML = cardHtml;
            const card = temp.firstElementChild;
            
            // Add to the grid
            relayGrid.appendChild(card);
            
            // Set initial state
            const control = document.getElementById(`${relayId}-control`);
            const statusElement = document.getElementById(`${relayId}-status`);
            
            if (control && statusElement) {
                control.checked = state === 'ON';
                control.disabled = !isOnline;
                
                updateStatusIndicator(statusElement, state);
                
                // Add event listener
                control.addEventListener('change', function() {
                    const state = this.checked ? 'ON' : 'OFF';
                    socket.emit('controlDevice', { 
                        deviceId: currentDevice, 
                        relay: relayId, 
                        state 
                    });
                    addLogEntry(`Requested ${currentDevice} ${relayId} change to: ${state}`);
                });
            }
            
            // Add disabled class if device is offline
            if (!isOnline) {
                card.classList.add('disabled');
            }
        }
    }
}

// Function to clear the relay grid
function clearRelayGrid() {
    while (relayGrid.firstChild) {
        relayGrid.removeChild(relayGrid.firstChild);
    }
}

// Function to update the status of a specific relay
function updateRelayStatus(relayId, state) {
    const control = document.getElementById(`${relayId}-control`);
    const statusElement = document.getElementById(`${relayId}-status`);
    
    if (control && statusElement) {
        control.checked = state === 'ON';
        updateStatusIndicator(statusElement, state);
    }
}

// Helper functions
function updateStatusIndicator(element, state) {
    element.textContent = state;
    element.className = '';

    if (state === 'ON') {
        element.classList.add('status-on');
    } else if (state === 'OFF') {
        element.classList.add('status-off');
    }
}

function addLogEntry(message, type = '') {
    const entry = document.createElement('div');
    entry.className = 'log-entry';

    if (type) {
        entry.classList.add(type);
    }

    const timestamp = document.createElement('span');
    timestamp.className = 'log-timestamp';
    const now = new Date();
    timestamp.textContent = now.toLocaleTimeString();

    const content = document.createElement('span');
    content.className = 'log-content';
    content.textContent = message;

    entry.appendChild(timestamp);
    entry.appendChild(content);

    logEntries.prepend(entry);

    // Limit the number of log entries to prevent the DOM from getting too large
    while (logEntries.children.length > 100) {
        logEntries.removeChild(logEntries.lastChild);
    }
}