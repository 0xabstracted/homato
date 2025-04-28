import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter BLE Client',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: HomeScreen(),
    );
  }
}

class HomeScreen extends StatefulWidget {
  @override
  _HomeScreenState createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  // BLE instance
  
  // Found devices list
  List<ScanResult> scanResults = [];
  
  // Scanning state
  bool isScanning = false;
  
  // Selected device
  BluetoothDevice? selectedDevice;
  
  // Connected device states
  bool isConnecting = false;
  bool isDisconnecting = false;
  
  // Stream subscription for scanning
  StreamSubscription<List<ScanResult>>? scanSubscription;

  @override
  void initState() {
    super.initState();
    // Init permissions
    _requestPermissions();
  }

  @override
  void dispose() {
    scanSubscription?.cancel();
    // Disconnect selected device if any
    selectedDevice?.disconnect();
    super.dispose();
  }

  // Request required permissions
  Future<void> _requestPermissions() async {
    // Request Bluetooth permissions
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetooth,
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();
    
    if (statuses.values.any((status) => status != PermissionStatus.granted)) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Permissions not granted'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  // Start scanning for BLE devices
  void startScan() {
    setState(() {
      scanResults = [];
      isScanning = true;
    });
    
    // Listen to scan results
    scanSubscription = FlutterBluePlus.scanResults.listen((results) {
      setState(() {
        scanResults = results;
      });
    }, onDone: () {
      setState(() {
        isScanning = false;
      });
    });
    
    // Start scanning
    FlutterBluePlus.startScan(timeout: Duration(seconds: 10));
    
    // After 10 seconds, stop scanning
    Future.delayed(Duration(seconds: 10), () {
      stopScan();
    });
  }

  // Stop scanning for BLE devices
  void stopScan() {
    FlutterBluePlus.stopScan();
    setState(() {
      isScanning = false;
    });
  }

  // Connect to a device
  Future<void> connectToDevice(BluetoothDevice device) async {
    setState(() {
      isConnecting = true;
      selectedDevice = device;
    });
    
    try {
      await device.connect();
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Connected to ${device.name}'),
          backgroundColor: Colors.green,
        ),
      );
      // Navigate to device screen
      Navigator.of(context).push(
        MaterialPageRoute(
          builder: (context) => DeviceScreen(device: device),
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to connect: $e'),
          backgroundColor: Colors.red,
        ),
      );
    } finally {
      setState(() {
        isConnecting = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('BLE Scanner'),
      ),
      body: Column(
        children: <Widget>[
          // Scanning status
          Container(
            padding: EdgeInsets.all(16),
            color: isScanning ? Colors.blue : Colors.grey,
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: <Widget>[
                Text(
                  isScanning ? 'Scanning...' : 'Not scanning',
                  style: TextStyle(
                    color: Colors.white,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Icon(
                  isScanning ? Icons.bluetooth_searching : Icons.bluetooth,
                  color: Colors.white,
                ),
              ],
            ),
          ),
          
          // Device list
          Expanded(
            child: ListView.builder(
              itemCount: scanResults.length,
              itemBuilder: (context, index) {
                ScanResult result = scanResults[index];
                return ListTile(
                  title: Text(
                    result.device.name.isEmpty
                        ? 'Unknown Device'
                        : result.device.name,
                  ),
                  subtitle: Text(result.device.id.id),
                  trailing: Text(result.rssi.toString() + ' dBm'),
                  onTap: () => connectToDevice(result.device),
                );
              },
            ),
          ),
        ],
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: isScanning ? stopScan : startScan,
        child: Icon(isScanning ? Icons.stop : Icons.search),
      ),
    );
  }
}

class DeviceScreen extends StatefulWidget {
  final BluetoothDevice device;

  const DeviceScreen({Key? key, required this.device}) : super(key: key);

  @override
  _DeviceScreenState createState() => _DeviceScreenState();
}

class _DeviceScreenState extends State<DeviceScreen> {
  // List of discovered services
  List<BluetoothService> services = [];
  
  // Loading state
  bool isLoading = true;

  @override
  void initState() {
    super.initState();
    // Discover services
    discoverServices();
  }

  // Discover device services
  Future<void> discoverServices() async {
    setState(() {
      isLoading = true;
    });
    
    try {
      services = await widget.device.discoverServices();
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to discover services: $e'),
          backgroundColor: Colors.red,
        ),
      );
    } finally {
      setState(() {
        isLoading = false;
      });
    }
  }

  // Read characteristic value
  Future<void> readCharacteristic(BluetoothCharacteristic characteristic) async {
    try {
      List<int> value = await characteristic.read();
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Read value: ${value.toString()}'),
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to read: $e'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  // Write to characteristic
  Future<void> writeCharacteristic(
      BluetoothCharacteristic characteristic, List<int> value) async {
    try {
      await characteristic.write(value);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Write successful'),
          backgroundColor: Colors.green,
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to write: $e'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  // Subscribe to characteristic notifications
  Future<void> subscribeToCharacteristic(
      BluetoothCharacteristic characteristic) async {
    try {
      await characteristic.setNotifyValue(true);
      characteristic.value.listen((value) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Notification: ${value.toString()}'),
          ),
        );
      });
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Subscribed to notifications'),
          backgroundColor: Colors.green,
        ),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to subscribe: $e'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  // Disconnect from device
  Future<void> disconnect() async {
    try {
      await widget.device.disconnect();
      Navigator.of(context).pop();
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Failed to disconnect: $e'),
          backgroundColor: Colors.red,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.device.name),
        actions: <Widget>[
          IconButton(
            icon: Icon(Icons.refresh),
            onPressed: discoverServices,
          ),
        ],
      ),
      body: isLoading
          ? Center(child: CircularProgressIndicator())
          : ListView.builder(
              itemCount: services.length,
              itemBuilder: (context, index) {
                BluetoothService service = services[index];
                return ExpansionTile(
                  title: Text('Service: ${service.uuid.toString()}'),
                  children: service.characteristics.map((characteristic) {
                    return ListTile(
                      title: Text('Characteristic: ${characteristic.uuid.toString()}'),
                      subtitle: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: <Widget>[
                          Text('Properties:'),
                          Row(
                            children: <Widget>[
                              // Properties buttons
                              if (characteristic.properties.read)
                                TextButton(
                                  child: Text('READ'),
                                  onPressed: () => readCharacteristic(characteristic),
                                ),
                              if (characteristic.properties.write)
                                TextButton(
                                  child: Text('WRITE'),
                                  onPressed: () => showDialog(
                                    context: context,
                                    builder: (context) => WriteDialog(
                                      onWrite: (value) => writeCharacteristic(
                                          characteristic, value),
                                    ),
                                  ),
                                ),
                              if (characteristic.properties.notify)
                                TextButton(
                                  child: Text('NOTIFY'),
                                  onPressed: () =>
                                      subscribeToCharacteristic(characteristic),
                                ),
                            ],
                          ),
                        ],
                      ),
                    );
                  }).toList(),
                );
              },
            ),
      floatingActionButton: FloatingActionButton(
        onPressed: disconnect,
        child: Icon(Icons.bluetooth_disabled),
        backgroundColor: Colors.red,
      ),
    );
  }
}

// Dialog for writing to characteristics
class WriteDialog extends StatefulWidget {
  final Function(List<int>) onWrite;

  const WriteDialog({Key? key, required this.onWrite}) : super(key: key);

  @override
  _WriteDialogState createState() => _WriteDialogState();
}

class _WriteDialogState extends State<WriteDialog> {
  final TextEditingController _controller = TextEditingController();

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: Text('Write Value'),
      content: TextField(
        controller: _controller,
        decoration: InputDecoration(
          labelText: 'Enter hex values (e.g., 01 02 FF)',
        ),
      ),
      actions: <Widget>[
        TextButton(
          child: Text('CANCEL'),
          onPressed: () => Navigator.of(context).pop(),
        ),
        TextButton(
          child: Text('WRITE'),
          onPressed: () {
            // Convert input hex string to list of integers
            List<int> value = _controller.text
                .split(' ')
                .map((hex) => int.parse(hex, radix: 16))
                .toList();
            widget.onWrite(value);
            Navigator.of(context).pop();
          },
        ),
      ],
    );
  }
}