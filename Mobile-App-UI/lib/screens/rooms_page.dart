import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';

class RoomsPage extends StatefulWidget {
  const RoomsPage({super.key});

  @override
  State<RoomsPage> createState() => _RoomsPageState();
}

class _RoomsPageState extends State<RoomsPage> {
  // Mock list of rooms for demonstration
  final List<Map<String, dynamic>> _rooms = [
    {
      'id': 'room_001',
      'name': 'Living Room',
      'deviceCount': 8,
      'temperature': '22°C',
      'humidity': '45%',
      'icon': Icons.living,
      'color': Colors.blue,
      'devices': ['Smart TV', 'Light Bulbs', 'Air Conditioner', 'Speaker'],
    },
    {
      'id': 'room_002',
      'name': 'Bedroom',
      'deviceCount': 5,
      'temperature': '20°C',
      'humidity': '50%',
      'icon': Icons.bed,
      'color': Colors.purple,
      'devices': ['Smart Lamp', 'Temperature Sensor', 'Blinds', 'Fan'],
    },
    {
      'id': 'room_003',
      'name': 'Kitchen',
      'deviceCount': 6,
      'temperature': '24°C',
      'humidity': '55%',
      'icon': Icons.kitchen,
      'color': Colors.orange,
      'devices': ['Smart Fridge', 'Oven', 'Dishwasher', 'Coffee Maker'],
    },
    {
      'id': 'room_004',
      'name': 'Bathroom',
      'deviceCount': 3,
      'temperature': '23°C',
      'humidity': '65%',
      'icon': Icons.bathroom,
      'color': Colors.teal,
      'devices': ['Smart Mirror', 'Water Heater', 'Ventilation Fan'],
    },
    {
      'id': 'room_005',
      'name': 'Garden',
      'deviceCount': 4,
      'temperature': '18°C',
      'humidity': '70%',
      'icon': Icons.grass,
      'color': Colors.green,
      'devices': ['Sprinkler System', 'Weather Station', 'Outdoor Lights'],
    },
    {
      'id': 'room_006',
      'name': 'Home Office',
      'deviceCount': 7,
      'temperature': '21°C',
      'humidity': '40%',
      'icon': Icons.computer,
      'color': Colors.indigo,
      'devices': ['Smart Desk', 'Monitor', 'Air Purifier', 'Webcam'],
    },
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Rooms'),
        actions: [
          IconButton(
            icon: const Icon(Icons.add),
            onPressed: () {
              // TODO: Add new room functionality
              ScaffoldMessenger.of(context).showSnackBar(
                const SnackBar(
                  content: Text('Add new room feature coming soon!'),
                ),
              );
            },
          ),
        ],
      ),
      body: Column(
        children: [
          // Header section
          Container(
            width: double.infinity,
            padding: const EdgeInsets.all(16.0),
            decoration: BoxDecoration(
              gradient: LinearGradient(
                colors: [Colors.blue.shade50, Colors.white],
                begin: Alignment.topCenter,
                end: Alignment.bottomCenter,
              ),
            ),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const Text(
                  'Your Smart Home',
                  style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: Colors.black87,
                  ),
                ),
                const SizedBox(height: 4),
                Text(
                  '${_rooms.length} rooms configured',
                  style: TextStyle(fontSize: 16, color: Colors.grey.shade600),
                ),
              ],
            ),
          ),

          // Rooms list
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(16.0),
              child: GridView.builder(
                gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
                  crossAxisCount: 2,
                  crossAxisSpacing: 12,
                  mainAxisSpacing: 12,
                  childAspectRatio: 0.85,
                ),
                itemCount: _rooms.length,
                itemBuilder: (context, index) {
                  final room = _rooms[index];

                  return Card(
                    elevation: 4,
                    shadowColor: room['color'].withOpacity(0.3),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(16),
                    ),
                    child: InkWell(
                      borderRadius: BorderRadius.circular(16),
                      onTap: () {
                        context.go('/rooms/devices');
                      },
                      child: Container(
                        padding: const EdgeInsets.all(16),
                        decoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(16),
                          gradient: LinearGradient(
                            colors: [
                              room['color'].withOpacity(0.1),
                              Colors.white,
                            ],
                            begin: Alignment.topLeft,
                            end: Alignment.bottomRight,
                          ),
                        ),
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            // Room icon and device count
                            Row(
                              mainAxisAlignment: MainAxisAlignment.spaceBetween,
                              children: [
                                Container(
                                  padding: const EdgeInsets.all(12),
                                  decoration: BoxDecoration(
                                    color: room['color'].withOpacity(0.2),
                                    borderRadius: BorderRadius.circular(12),
                                  ),
                                  child: Icon(
                                    room['icon'],
                                    color: room['color'],
                                    size: 24,
                                  ),
                                ),
                                Container(
                                  padding: const EdgeInsets.symmetric(
                                    horizontal: 8,
                                    vertical: 4,
                                  ),
                                  decoration: BoxDecoration(
                                    color: Colors.grey.shade100,
                                    borderRadius: BorderRadius.circular(12),
                                  ),
                                  child: Text(
                                    '${room['deviceCount']} devices',
                                    style: TextStyle(
                                      fontSize: 12,
                                      color: Colors.grey.shade600,
                                      fontWeight: FontWeight.w500,
                                    ),
                                  ),
                                ),
                              ],
                            ),

                            const SizedBox(height: 12),

                            // Room name
                            Text(
                              room['name'],
                              style: const TextStyle(
                                fontSize: 18,
                                fontWeight: FontWeight.bold,
                                color: Colors.black87,
                              ),
                            ),

                            const SizedBox(height: 8),

                            // Temperature and humidity
                            Row(
                              children: [
                                Icon(
                                  Icons.thermostat,
                                  size: 16,
                                  color: Colors.grey.shade600,
                                ),
                                const SizedBox(width: 4),
                                Text(
                                  room['temperature'],
                                  style: TextStyle(
                                    fontSize: 14,
                                    color: Colors.grey.shade600,
                                  ),
                                ),
                                const SizedBox(width: 12),
                                Icon(
                                  Icons.water_drop,
                                  size: 16,
                                  color: Colors.grey.shade600,
                                ),
                                const SizedBox(width: 4),
                                Text(
                                  room['humidity'],
                                  style: TextStyle(
                                    fontSize: 14,
                                    color: Colors.grey.shade600,
                                  ),
                                ),
                              ],
                            ),

                            const Spacer(),

                            // Top devices preview
                            Text(
                              room['devices'].take(2).join(', '),
                              style: TextStyle(
                                fontSize: 12,
                                color: Colors.grey.shade500,
                              ),
                              maxLines: 1,
                              overflow: TextOverflow.ellipsis,
                            ),
                          ],
                        ),
                      ),
                    ),
                  );
                },
              ),
            ),
          ),
        ],
      ),
    );
  }
}
