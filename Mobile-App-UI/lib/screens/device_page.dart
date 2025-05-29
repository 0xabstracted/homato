import 'package:flutter/material.dart';

class DevicePage extends StatefulWidget {
  final String id;

  const DevicePage({super.key, required this.id});

  @override
  State<DevicePage> createState() => _DevicePageState();
}

class _DevicePageState extends State<DevicePage> {
  // Mock device data based on ID
  Map<String, dynamic>? _device;
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadDeviceData();
  }

  void _loadDeviceData() {
    // Simulate loading device data
    Future.delayed(const Duration(seconds: 1), () {
      setState(() {
        _device = _getDeviceById(widget.id);
        _isLoading = false;
      });
    });
  }

  Map<String, dynamic>? _getDeviceById(String id) {
    final devices = {
      'device_001': {
        'id': 'device_001',
        'name': 'Smart Light Bulb',
        'type': 'Light',
        'status': 'Connected',
        'batteryLevel': 85,
        'lastSeen': 'Just now',
        'icon': Icons.lightbulb,
        'room': 'Living Room',
        'brightness': 75,
        'color': 'Warm White',
        'powerConsumption': '8W',
      },
      'device_002': {
        'id': 'device_002',
        'name': 'Temperature Sensor',
        'type': 'Sensor',
        'status': 'Connected',
        'batteryLevel': 67,
        'lastSeen': '2 minutes ago',
        'icon': Icons.thermostat,
        'room': 'Bedroom',
        'temperature': '22.5°C',
        'humidity': '45%',
        'powerConsumption': '2W',
      },
      'device_003': {
        'id': 'device_003',
        'name': 'Door Lock',
        'type': 'Security',
        'status': 'Disconnected',
        'batteryLevel': 23,
        'lastSeen': '1 hour ago',
        'icon': Icons.lock,
        'room': 'Front Door',
        'lockStatus': 'Locked',
        'accessLog': '5 entries today',
        'powerConsumption': '1W',
      },
      'device_004': {
        'id': 'device_004',
        'name': 'Motion Detector',
        'type': 'Sensor',
        'status': 'Connected',
        'batteryLevel': 91,
        'lastSeen': 'Just now',
        'icon': Icons.motion_photos_on,
        'room': 'Hallway',
        'motionDetected': false,
        'sensitivity': 'Medium',
        'powerConsumption': '3W',
      },
    };

    return devices[id];
  }

  @override
  Widget build(BuildContext context) {
    if (_isLoading) {
      return Scaffold(
        appBar: AppBar(title: const Text('Device Details')),
        body: const Center(child: CircularProgressIndicator()),
      );
    }

    if (_device == null) {
      return Scaffold(
        appBar: AppBar(title: const Text('Device Not Found')),
        body: const Center(child: Text('Device not found')),
      );
    }

    final isConnected = _device!['status'] == 'Connected';

    return Scaffold(
      appBar: AppBar(
        title: Text(_device!['name']),
        actions: [
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () {
              // Handle device settings
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Device header
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Row(
                  children: [
                    CircleAvatar(
                      radius: 30,
                      backgroundColor: isConnected ? Colors.green : Colors.grey,
                      child: Icon(
                        _device!['icon'],
                        size: 30,
                        color: Colors.white,
                      ),
                    ),
                    const SizedBox(width: 16),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            _device!['name'],
                            style: const TextStyle(
                              fontSize: 20,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          Text('Type: ${_device!['type']}'),
                          Text('Room: ${_device!['room']}'),
                          Row(
                            children: [
                              Icon(
                                Icons.circle,
                                size: 12,
                                color: isConnected ? Colors.green : Colors.red,
                              ),
                              const SizedBox(width: 4),
                              Text(_device!['status']),
                            ],
                          ),
                        ],
                      ),
                    ),
                    Column(
                      children: [
                        Icon(
                          Icons.battery_std,
                          color:
                              _device!['batteryLevel'] > 30
                                  ? Colors.green
                                  : Colors.red,
                        ),
                        Text('${_device!['batteryLevel']}%'),
                      ],
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 20),

            // Device details
            const Text(
              'Device Information',
              style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),

            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  children: [
                    _buildInfoRow('Device ID', _device!['id']),
                    const Divider(),
                    _buildInfoRow('Last Seen', _device!['lastSeen']),
                    const Divider(),
                    _buildInfoRow(
                      'Power Consumption',
                      _device!['powerConsumption'],
                    ),
                    if (_device!['type'] == 'Light') ...[
                      const Divider(),
                      _buildInfoRow('Brightness', '${_device!['brightness']}%'),
                      const Divider(),
                      _buildInfoRow('Color', _device!['color']),
                    ],
                    if (_device!['type'] == 'Sensor' &&
                        _device!['temperature'] != null) ...[
                      const Divider(),
                      _buildInfoRow('Temperature', _device!['temperature']),
                      const Divider(),
                      _buildInfoRow('Humidity', _device!['humidity']),
                    ],
                    if (_device!['type'] == 'Security') ...[
                      const Divider(),
                      _buildInfoRow('Lock Status', _device!['lockStatus']),
                      const Divider(),
                      _buildInfoRow('Access Log', _device!['accessLog']),
                    ],
                  ],
                ),
              ),
            ),

            const SizedBox(height: 20),

            // Action buttons
            Row(
              children: [
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed:
                        isConnected
                            ? () {
                              // Handle disconnect
                              ScaffoldMessenger.of(context).showSnackBar(
                                const SnackBar(
                                  content: Text('Device disconnected'),
                                ),
                              );
                            }
                            : () {
                              // Handle connect
                              ScaffoldMessenger.of(context).showSnackBar(
                                const SnackBar(
                                  content: Text('Connecting to device...'),
                                ),
                              );
                            },
                    icon: Icon(
                      isConnected ? Icons.bluetooth_disabled : Icons.bluetooth,
                    ),
                    label: Text(isConnected ? 'Disconnect' : 'Connect'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: isConnected ? Colors.red : Colors.blue,
                      foregroundColor: Colors.white,
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: () {
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(content: Text('Device removed')),
                      );
                      Navigator.of(context).pop();
                    },
                    icon: const Icon(Icons.delete),
                    label: const Text('Remove'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.grey,
                      foregroundColor: Colors.white,
                    ),
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInfoRow(String label, String value) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text(label, style: const TextStyle(fontWeight: FontWeight.w500)),
        Text(value),
      ],
    );
  }
}
