import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';

class DevicesPage extends StatefulWidget {
  const DevicesPage({super.key});

  @override
  State<DevicesPage> createState() => _DevicesPageState();
}

class _DevicesPageState extends State<DevicesPage> {
  // Mock list of devices for demonstration
  final List<Map<String, dynamic>> _devices = [
    {
      'id': 'device_001',
      'name': 'Smart Light Bulb',
      'type': 'Light',
      'status': 'Connected',
      'batteryLevel': 85,
      'lastSeen': 'Just now',
      'icon': Icons.lightbulb,
    },
    {
      'id': 'device_002',
      'name': 'Temperature Sensor',
      'type': 'Sensor',
      'status': 'Connected',
      'batteryLevel': 67,
      'lastSeen': '2 minutes ago',
      'icon': Icons.thermostat,
    },
    {
      'id': 'device_003',
      'name': 'Door Lock',
      'type': 'Security',
      'status': 'Disconnected',
      'batteryLevel': 23,
      'lastSeen': '1 hour ago',
      'icon': Icons.lock,
    },
    {
      'id': 'device_004',
      'name': 'Motion Detector',
      'type': 'Sensor',
      'status': 'Connected',
      'batteryLevel': 91,
      'lastSeen': 'Just now',
      'icon': Icons.motion_photos_on,
    },
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Devices'),
        centerTitle: true,
        actions: [
          IconButton(
            icon: const Icon(Icons.add),
            onPressed: () {
              // Navigate to BLE screen to add new devices
              context.go('/ble');
            },
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Connected Devices (${_devices.where((device) => device['status'] == 'Connected').length})',
              style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 16),
            Expanded(
              child: ListView.builder(
                itemCount: _devices.length,
                itemBuilder: (context, index) {
                  final device = _devices[index];
                  final isConnected = device['status'] == 'Connected';

                  return Card(
                    margin: const EdgeInsets.only(bottom: 12),
                    child: ListTile(
                      leading: CircleAvatar(
                        backgroundColor:
                            isConnected ? Colors.green : Colors.grey,
                        child: Icon(device['icon'], color: Colors.white),
                      ),
                      title: Text(
                        device['name'],
                        style: const TextStyle(fontWeight: FontWeight.w600),
                      ),
                      subtitle: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text('Type: ${device['type']}'),
                          Text('Status: ${device['status']}'),
                          Text('Last seen: ${device['lastSeen']}'),
                        ],
                      ),
                      trailing: Column(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Icon(
                            Icons.battery_std,
                            color:
                                device['batteryLevel'] > 30
                                    ? Colors.green
                                    : Colors.red,
                          ),
                          Text('${device['batteryLevel']}%'),
                        ],
                      ),
                      onTap: () {
                        context.go('/rooms/devices/${device['id']}');
                      },
                    ),
                  );
                },
              ),
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {
          context.go('/ble');
        },
        child: const Icon(Icons.add),
        tooltip: 'Add Device',
      ),
    );
  }
}
