// Connect to Socket.io
const socket = io();

// DOM Elements
const connectionIndicator = document.getElementById('connection-indicator');
const deviceConnectionStatus = document.getElementById('device-connection-status');
const switchControl = document.getElementById('switch-control');
const lightControl = document.getElementById('light-control');
const fanControl = document.getElementById('fan-control');
const tubelightControl = document.getElementById('tubelight-control');
const bedlightControl = document.getElementById('bedlight-control');
const falseceilingControl = document.getElementById('falseceiling-control');
const acControl = document.getElementById('ac-control');
const switchportControl = document.getElementById('switchport-control');
const switchStatus = document.getElementById('switch-status');
const lightStatus = document.getElementById('light-status');
const fanStatus = document.getElementById('fan-status');
const tubelightStatus = document.getElementById('tubelight-status');
const bedlightStatus = document.getElementById('bedlight-status');
const falseceilingStatus = document.getElementById('falseceiling-status');
const acStatus = document.getElementById('ac-status');
const switchportStatus = document.getElementById('switchport-status');
const switchCard = document.getElementById('switch-card');
const lightCard = document.getElementById('light-card');
const fanCard = document.getElementById('fan-card');
const tubelightCard = document.getElementById('tubelight-card');
const bedlightCard = document.getElementById('bedlight-card');
const falseceilingCard = document.getElementById('falseceiling-card');
const acCard = document.getElementById('ac-card');
const switchportCard = document.getElementById('switchport-card');
const logEntries = document.getElementById('log-entries');

// Device connection state
let deviceConnected = false;

// Initialize UI state
switchControl.checked = false;
lightControl.checked = false;
fanControl.checked = false;
tubelightControl.checked = false;
bedlightControl.checked = false;
falseceilingControl.checked = false;
acControl.checked = false;
switchportControl.checked = false;
updateStatusIndicator(switchStatus, 'Unknown');
updateStatusIndicator(lightStatus, 'Unknown');
updateStatusIndicator(fanStatus, 'Unknown');
updateStatusIndicator(tubelightStatus, 'Unknown');
updateStatusIndicator(bedlightStatus, 'Unknown');
updateStatusIndicator(falseceilingStatus, 'Unknown');
updateStatusIndicator(acStatus, 'Unknown');
updateStatusIndicator(switchportStatus, 'Unknown');
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
    const deviceMap = {
        [TOPICS.switch]: 'switch',
        [TOPICS.light]: 'light',
        [TOPICS.fan]: 'fan',
        [TOPICS.tubelight]: 'tubelight',
        [TOPICS.bedlight]: 'bedlight',
        [TOPICS.falseceiling]: 'falseceiling',
        [TOPICS.ac]: 'ac',
        [TOPICS.switchport]: 'switchport'
    };

    const deviceName = deviceMap[data.topic] ||
        Object.keys(deviceMap).find(key => data.topic.includes(key.split('/')[1]));

    if (deviceName) {
        updateStatus(deviceName, data.state);
        addLogEntry(`${deviceName.charAt(0).toUpperCase() + deviceName.slice(1)} state updated to: ${data.state}`,
            data.state === 'ON' ? 'success' : '');
    }
});

// MQTT Topics mirror
const TOPICS = {
    switch: 'home/switch',
    light: 'home/light',
    fan: 'home/fan',
    tubelight: 'home/tubelight',
    bedlight: 'home/bedlight',
    falseceiling: 'home/falseceiling',
    ac: 'home/ac',
    switchport: 'home/switchport'
};

// Device connection update events
socket.on('deviceConnectionUpdate', (data) => {
    updateDeviceConnectionStatus(data.connected);
    addLogEntry(
        `Device is now ${data.connected ? 'connected' : 'disconnected'}`,
        data.connected ? 'success' : 'error'
    );
});

// Set up event listeners for all controls
function setupControlListener(control, device) {
    control.addEventListener('change', () => {
        if (!deviceConnected) {
            // Prevent toggle if device is offline
            control.checked = !control.checked;
            addLogEntry(`Cannot control ${device}: Device is offline`, 'error');
            return;
        }

        const state = control.checked ? 'ON' : 'OFF';
        socket.emit('controlDevice', { device, state });
        addLogEntry(`Requested ${device} change to: ${state}`);
    });
}

// Control event listeners
setupControlListener(switchControl, 'switch');
setupControlListener(lightControl, 'light');
setupControlListener(fanControl, 'fan');
setupControlListener(tubelightControl, 'tubelight');
setupControlListener(bedlightControl, 'bedlight');
setupControlListener(falseceilingControl, 'falseceiling');
setupControlListener(acControl, 'ac');
setupControlListener(switchportControl, 'switchport');

// Helper functions
function updateUIState(data) {
    const devices = [
        'switch', 'light', 'fan', 'tubelight',
        'bedlight', 'falseceiling', 'ac', 'switchport'
    ];

    devices.forEach(device => {
        if (data[device]) {
            updateStatus(device, data[device]);
        }
    });
}

function updateStatus(device, state) {
    const isOn = state === 'ON';
    const controls = {
        'switch': switchControl,
        'light': lightControl,
        'fan': fanControl,
        'tubelight': tubelightControl,
        'bedlight': bedlightControl,
        'falseceiling': falseceilingControl,
        'ac': acControl,
        'switchport': switchportControl
    };

    const statuses = {
        'switch': switchStatus,
        'light': lightStatus,
        'fan': fanStatus,
        'tubelight': tubelightStatus,
        'bedlight': bedlightStatus,
        'falseceiling': falseceilingStatus,
        'ac': acStatus,
        'switchport': switchportStatus
    };

    if (controls[device] && statuses[device]) {
        controls[device].checked = isOn;
        updateStatusIndicator(statuses[device], state);
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

    // List all device cards to update
    const deviceCards = [
        switchCard, lightCard, fanCard, tubelightCard,
        bedlightCard, falseceilingCard, acCard, switchportCard
    ];

    // Update device cards
    deviceCards.forEach(card => {
        if (card) card.classList.toggle('disabled', !connected);
    });

    // List all device controls to update
    const deviceControls = [
        switchControl, lightControl, fanControl, tubelightControl,
        bedlightControl, falseceilingControl, acControl, switchportControl
    ];

    // Enable/disable controls based on connection
    deviceControls.forEach(control => {
        if (control) control.disabled = !connected;
    });
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